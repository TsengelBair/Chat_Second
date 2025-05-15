#include <QTcpSocket>

#include "server.h"
#include "types.h"
#include "validator.h"
#include "serializer.h"
#include "dbhandler.h"
#include "packetbuilder.h"

#include "../ProtoFiles/IAuthResponse.pb.h"

Server::Server(QHostAddress address, qint16 port)
{
    if (this->listen(address, port)){
        qDebug() << "Listen";
    } else {
        qDebug() << "Error: " << errorString();
    }
}

Server::~Server()
{
    close();
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);

    qDebug() << "Client connected " << socketDescriptor;

    connect(socket, &QTcpSocket::readyRead, this, &Server::handleIncommingDataFromClient);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    m_sockets.insert(socketDescriptor, socket);
}

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

void Server::handleFindUsersRequest(const QByteArray &packetData, qintptr socketDescriptor)
{
    QString loginToFind = Serializer::deserializeFindUserRequest(packetData);
    if (loginToFind.isEmpty()) {
        qDebug() << "Передан пустой логин";
        return;
    }

    QList<QPair<int, QString>> usersWithSimilarLogin = DbHandler::getInstance()->findUsersWithSimilarLogin(loginToFind);

    QByteArray data = Serializer::serializeFindedUsersResponse(usersWithSimilarLogin);
    QByteArray packet = PacketBuilder::createPacketToSend(data, ResponseType::RESPONSE_FIND_USERS);

    sendToClient(packet, socketDescriptor);
}


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

void Server::handleGetRequest(const QByteArray &packetData, qintptr socketDescriptor)
{
    GetRequestBody getReqBody = Serializer::deserializeGetRequest(packetData);

    bool dataForUserIsEmpty = false;

    /// получаем список чатов пользователя
    QVector<Chat> chats = DbHandler::getInstance()->loadUserChats(getReqBody.userId, getReqBody.chatsLimit);
    if (chats.empty()) {
        dataForUserIsEmpty = true;
    } else {
        /// каждый чат заполняем соответствующими сообщениями
        for (Chat &chat : chats) {
            chat.messages = DbHandler::getInstance()->loadChatMessages(chat.chatId, getReqBody.messagesInChatLimit);
        }
    }

    QByteArray data = Serializer::serializeGetResponse(chats, dataForUserIsEmpty);
    QByteArray packet = PacketBuilder::createPacketToSend(data, ResponseType::RESPONSE_GET_DEFAULT_DATA);

    sendToClient(packet, socketDescriptor);
}

void Server::sendToClient(const QByteArray &packet, qintptr socketDescriptor)
{
    QTcpSocket* socket = m_sockets.value(socketDescriptor);
    if (socket) {
        socket->write(packet);
        socket->flush();
    }
}

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

