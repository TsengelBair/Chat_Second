**На данный момент реализовано общение клиента и сервера по следующему протоколу**

Первые 4 байта резервируются под длину сообщения, пятый байт - это контрольная сумма, которая считается через XOR и шестой байт зарезервирован для хранения идентификатора запроса/ответа.

Вшивание шестого байта необходимо для идентификации пришедшего запроса/ответа поскольку общение клиента и сервера осуществляется через protobuf.

1. Как только на сервер/клиент приходит запрос/ответ (через сокет), происходит валидация пакета (сравнивается длина, указанная в первых 4 байт с фактическим размером, считается XOR)

2. Если пакет валиден, из шестого байта извлекается тип запроса, т.к. для десерилизации нужно знать на основе какого прото файла (pb хэдера) десерилизовать данные.


### Server

Начнем с реализации сервера, создав соответствующий класс Server, унаследованный от QTcpServer (для того чтобы переопределить виртуальный метод обработки входящего соединения incomingConnection)

Но перед этим запустим сервер на 5000 порту, указав localhost (порт и хост заданы в main.cpp)

```c++
Server::Server(QHostAddress address, qint16 port)
{
    if (this->listen(address, port)){
        qDebug() << "Listen";
    } else {
        qDebug() << "Error: " << errorString();
    }
}
```

```c++
#include <QCoreApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server(QHostAddress::LocalHost, 5000);

    return a.exec();
}
```
После того как сервер запущен, переопределим виртуальный метод incomingConnection, выступающий в кач-ве обработчика входящего соединения, параметром метод ожидает дескриптор сокета клиента (сам дескриптор генерируется на клиенте автоматически)

Внутри метода сохраним объект входящяего сокета в словаре под ключом в виде переданного дескриптора и подключим сигналы к соответствующим слотам.

Как только клиент отправит данные, сработает сигнал QTcpSocket::readyRead, сигнализирующий о том, что в сокете появились данные, доступные для чтения.


```c++
void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);

    qDebug() << "Client connected " << socketDescriptor;

    connect(socket, &QTcpSocket::readyRead, this, &Server::handleIncommingDataFromClient);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    m_sockets.insert(socketDescriptor, socket);
}
```

В слоте обработчике handleIncommingDataFromClient необходимо:

1. Проверить пришедшее количество байт, если пришло меньше 6 байт, пакет 100% не валиден
2. Провалидировать пакет, а именно:
    - извлечь из пакета указанную длину данных, вшитую в первые 4 байта и сравнить с фактической длинной данных (фактические данные идут после 6 байта, просто взять их размер)  
    - извлечь контрольную сумму, указанную в 5 байте, пройтись XOR по фактическим данным и сравнить посчитанную длину с указанной
3. Извлечь тип запроса из шестого байта и в зависимости от типа запроса вызвать соответствующий метод обработки

P.S.

Первые два условия - доп проверка (необязательно)

```c++
void Server::handleIncommingDataFromClient()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qDebug() << "Ошибка каста в socket";
        return;
    }

    /// проверяем, есть ли этот сокет в списке подключённых клиентов
    qintptr descriptor = socket->socketDescriptor();
    if (!m_sockets.contains(descriptor)) {
        qDebug() << "Попытка доступа от неподключённого клиента. Descriptor:" << descriptor
                 << "IP:" << socket->peerAddress().toString()
                 << "Port:" << socket->peerPort();

        socket->disconnectFromHost();
        return;
    }

    if (socket->bytesAvailable() < Validator::headerSize) {
        qDebug() << "Пришло меньше 6 байт";
        return;
    }

    QByteArray packetReceived = socket->readAll();

    bool isValid = Validator::validatePacket(packetReceived);
    if (!isValid) {
        qDebug() << "Ошибка при валидации пакета";
        return;
    }

    /// извлекаем тип запроса
    RequestType reqType = Validator::getRequestType(packetReceived);
    if (reqType == RequestType::REQUEST_LOGIN || reqType == RequestType::REQUEST_REGISTER) {
        handleAuthRequest(packetReceived.mid(Validator::headerSize), reqType, descriptor);
    } else if (reqType == RequestType::REQUEST_GET_DEFAULT_DATA) {
        handleGetRequest(packetReceived.mid(Validator::headerSize), descriptor);
    } else if (reqType == RequestType::REQUEST_FIND_USERS) {
        handleFindUsersRequest(packetReceived.mid(Validator::headerSize), descriptor);
    }
}
```

Класс Validator является статическим классом без конструктора и деструктора, т.к. выполняет утилитарную функцию

```c++
#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QByteArray>
#include "types.h"

class Validator
{
public:
    Validator() = delete;
    Validator(const Validator&) = delete;
    Validator& operator=(const Validator&) = delete;

    static bool validatePacket(const QByteArray &packet);
    static uint8_t calcCrc(const QByteArray &packetData);
    static RequestType getRequestType(const QByteArray &packet);

    static constexpr int headerSize = 6;
};

#endif // VALIDATOR_H
```
Валидация пакета по указанному алгоритму
```c++
bool Validator::validatePacket(const QByteArray &packet)
{
    /// Извлекаем первые 5 байт (4 - размер, 1 - CRC)
    QByteArray header = packet.left(headerSize - 1);
    QDataStream ds(&header, QIODevice::ReadOnly);

    qint32 dataSizeFromPacket;
    qint8 crcFromPacket;

    ds >> dataSizeFromPacket >> crcFromPacket;

    /// Получаем данные (всё, что после 6 байт)
    QByteArray packetData = packet.mid(headerSize);
    if (packetData.size() != dataSizeFromPacket) {
        qDebug() << "Указанный в заголовке размер данных не совпал с фактическим размером";
        return false;
    }

    /// Проверяем контрольную сумму
    uint8_t crc = calcCrc(packetData);
    if (static_cast<uint8_t>(crcFromPacket) != crc) {
        qDebug() << "CRC mismatch!";
        return false;
    }

    return true;
}

uint8_t Validator::calcCrc(const QByteArray &packetData)
{
    uint8_t crc = 0;
    for (const char c : packetData) {
        crc ^= static_cast<uint8_t>(c);
    }
    return crc;
}

RequestType Validator::getRequestType(const QByteArray &packet)
{
    return static_cast<RequestType>(packet.at(5));  /// 6-й байт (индекс 5)
}
```

**Рассмотрим обработку запроса авторизации**

Запрос авторизации представлен следующим прото файлом 

```proto
syntax = "proto3";

message IAuthRequest {
    string login = 1;
    string password = 2;
}
```

Этот файл необходимо скомпилировать, чтобы получить хэдер и исполняемый pb файлы, на основе которых и будет производиться серилизация/десерилизация (полученные хэдер и cpp должны быть доступны как на клиенте, так и на сервере)

```proto
protoc --cpp_out=.имя_прото_файла
```

Данный запрос будет преобразован на клиенте в массив байт (так как все данные по сокету передаются именно в таком виде) и упакован в пакет по указанному ранее протоколу

Также создадим прото файл, для представления ответа сервера, сам ответ - это код статуса и id пользователя (т.к. при успешной регистрации/авторизации необходимо в ответ сервера вшить id пользователя из бд)

```proto
syntax = "proto3";

message IAuthResponse {
    int32 code = 1;
    int32 userId = 2;
}
```

В самом методе выполним следующее:
1. Десериализуем ответ для того чтобы извлечь отправленные данные (в данном случае логин и пароль)
2. Произведем необходимые действия с извлеченными данными (в данном случае передадим их в класс DbHandler, для того чтобы зарегистрировать/авторизовать пользователя) и сформируем объект ответа
3. Сериализуем объект ответа
4. Упакуем ответ в пакет по указанному ранее протоколу и отправим клиенту, с которого пришел запрос (для этого и передается параметром socketDescriptor)

```c++
void Server::handleAuthRequest(const QByteArray &packetData, const RequestType &reqType, qintptr socketDescriptor)
{
    QPair<QString, QString> credentials = Serializer::deserializeAuthRequest(packetData);
    if (credentials.first.isEmpty() || credentials.second.isEmpty()) {
        qDebug() << "Логин или пароль пустые";
        return;
    }

    const QString& login = credentials.first;
    const QString& password = credentials.second;

    IAuthResponse response; /// объект ответа, который сериализуем
    ResponseType responseType; /// тип ответа, который положим в шестой байт tcp пакета

    switch (reqType) {
        case RequestType::REQUEST_REGISTER:
            handleRegistration(login, password, &response);
            responseType = RESPONSE_REGISTER;
            break;

        case RequestType::REQUEST_LOGIN:
            handleLogin(login, password, &response);
            responseType = RESPONSE_LOGIN;
            break;

        default:
            qDebug() << "Unknown request type:" << reqType;
            return;
    }

    QByteArray data = Serializer::serializeAuthResponse(&response);
    QByteArray packet = PacketBuilder::createPacketToSend(data, responseType);

    sendToClient(packet, socketDescriptor);
}
```

Сериализация/Десериализация будут производиться статическим классом Serializer

```c++
class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    static QByteArray serializeAuthResponse(IAuthResponse* response);
    static QPair<QString, QString> deserializeAuthRequest(const QByteArray &packetData);
};
```

Метод извлечения логина и пароля

```c++
QPair<QString, QString> Serializer::deserializeAuthRequest(const QByteArray &packetData)
{
    IAuthRequest authReq;
    if (!authReq.ParseFromString(packetData.toStdString())) {
        qDebug() << "Error while deserialize auth request";
        return QPair<QString, QString>();
    }

    QString login = QString::fromStdString(authReq.login());
    QString password = QString::fromStdString(authReq.password());

    return qMakePair(login, password);
}
```

В зависимости от типа запрос, указанного в шестом байте передаем логин и пароль в методы класса DbHandler (реализован через singleton, поэтому перед обращением к его методам, предварительно получаем указатель на единственный объект класса), также передаем указатель на объект ответа, т.к. формирование объекта ответа зависит от того, в каком методе обрабатывается запрос

```c++
void Server::handleRegistration(const QString &login, const QString &password, IAuthResponse *response)
{
    /// уже существует
    if (DbHandler::getInstance()->userExist(login)) {
        response->set_code(409);
        response->set_userid(-1);

        qDebug() << "Registration failed: user already exists";
        return;
    }

    /// добавляем в бд
    int newUserId = DbHandler::getInstance()->addUser(login, password);
    if (newUserId > 0) {
        response->set_code(200);
        response->set_userid(newUserId);
        qDebug() << "User registered successfully, ID:" << newUserId;
    } else {
        response->set_code(500); /// Internal Server Error
        response->set_userid(-1);
        qDebug() << "Registration failed: database error";
    }
}

void Server::handleLogin(const QString &login, const QString &password, IAuthResponse *response)
{
    /// не найден
    if (!DbHandler::getInstance()->userExist(login)) {
        response->set_code(404);
        response->set_userid(-1);

        qDebug() << "Login failed: user not found";
        return;
    }

    /// пароли не совпали
    if (!DbHandler::getInstance()->checkPassword(login, password)) {
        response->set_code(403); /// auth failed
        response->set_userid(-1);

        qDebug() << "Login failed: invalid password";
        return;
    }

    int userId = DbHandler::getInstance()->getUserId(login);
    response->set_code(201);
    response->set_userid(userId);

    qDebug() << "User logged in successfully, ID:" << userId;
}
```

После формирования объекта ответа сериализуем его (приводим к виду массива байт), формируем пакет и отправляем клиенту

```c++
QByteArray data = Serializer::serializeAuthResponse(&response);
QByteArray packet = PacketBuilder::createPacketToSend(data, responseType);

sendToClient(packet, socketDescriptor);
```

Формирование пакета осуществляется через статический класс PacketBuilder

```c++
class PacketBuilder
{
public:
    /// статик класс
    PacketBuilder() = delete;
    PacketBuilder(const PacketBuilder&) = delete;
    PacketBuilder& operator=(const PacketBuilder&) = delete;

    static QByteArray createPacketToSend(const QByteArray &data, const ResponseType &responseType);
};
```

```c++
QByteArray PacketBuilder::createPacketToSend(const QByteArray &data, const ResponseType &responseType)
{
    QByteArray packet;

    int packetSize = data.size();

    /// первые 4 байта размер данных
    QDataStream out(&packet, QIODevice::WriteOnly);
    out << qint32(packetSize);

    /// пятый байт контрольная сумма
    uint8_t crc = Validator::calcCrc(data);
    packet.append(static_cast<char>(crc));

    /// шестой тип запроса
    packet.append(static_cast<uint8_t>(responseType));

    /// сами данные
    packet.append(data);
    return packet;
}
```

Отправка клиенту

```c++
void Server::sendToClient(const QByteArray &packet, qintptr socketDescriptor)
{
    QTcpSocket* socket = m_sockets.value(socketDescriptor);
    if (socket) {
        socket->write(packet);
        socket->flush();
    }
}
```