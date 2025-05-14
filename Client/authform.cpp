#include <QMessageBox>

#include "authform.h"
#include "ui_authform.h"
#include "validator.h"
#include "serializer.h"
#include "packetbuilder.h"
#include "mainwindow.h"

AuthForm::AuthForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuthForm)
{
    ui->setupUi(this);

    m_socket.reset(new QTcpSocket());
    connect(m_socket.data(), &QTcpSocket::readyRead, this, &AuthForm::handleIncommingDataFromServer);

    connectToServer();

    connect(ui->pushButton, &QPushButton::clicked, this, &AuthForm::sendAuthReq);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &AuthForm::sendAuthReq);
}

AuthForm::~AuthForm()
{
    delete ui;
}

void AuthForm::connectToServer()
{
    m_socket->connectToHost("127.0.0.1", 5000);
}

void AuthForm::handleIncommingDataFromServer()
{
    while (m_socket->bytesAvailable()) {
        QByteArray packetReceived = m_socket->readAll();
        if (packetReceived.size() < Validator::headerSize) {
            qDebug() << "Пришло меньше 6 байт";
            return;
        }

        bool isValid = Validator::validatePacket(packetReceived);
        if (!isValid) {
            qDebug() << "Ошибка при валидации пакета";
            return;
        }

        ResponseType responseType = Validator::getResponseType(packetReceived);
        qDebug() << "Тип ответа: " << responseType;

        if (responseType == ResponseType::RESPONSE_LOGIN || responseType == ResponseType::RESPONSE_REGISTER) {
            handleAuthResponse(packetReceived.mid(Validator::headerSize), responseType);
        } else if (responseType == ResponseType::RESPONSE_GET_CHATS_EMPTY) {
            if (!mainWindow) {
                mainWindow.reset(new MainWindow(m_socket));
                connect(this, &AuthForm::signalFoundUsers, mainWindow.data(), &MainWindow::handleUsersFound);

                mainWindow->show();
                this->hide();
            }
            /// пока хардкодом, в будущем вынести handleIncommingDataFromServer в соответствующий класс
        } else if (responseType == ResponseType::RESPONSE_FIND_USERS && mainWindow) {
            QList<QString> foundUsers = Serializer::deserializeFoundUsersResponse(packetReceived.mid(Validator::headerSize));
            emit signalFoundUsers(foundUsers);
        }
    }
}

void AuthForm::handleAuthResponse(const QByteArray &data, const ResponseType &responseType)
{
    QPair<int, int> serverResponse = Serializer::deserializeAuthResponse(data);

    int serverResponseCode = serverResponse.first;
    int userId = serverResponse.second;

    if (serverResponseCode == 409) {
        QMessageBox::warning(this, "Предупреждение", "Registration failed: user already exists");
    } else if (serverResponseCode == 200) {
        sendGetReq(userId);
        return;
    } else if (serverResponseCode == 500) {
        QMessageBox::warning(this, "Предупреждение", "Registration failed: server error");
    } else if (serverResponseCode == 404) {
        QMessageBox::warning(this, "Предупреждение", "Login failed: user not found");
    } else if (serverResponseCode == 403) {
        QMessageBox::warning(this, "Предупреждение", "Login failed: invalid password");
    } else if (serverResponseCode == 201) {
        sendGetReq(userId);
        return;
    }

    qDebug() << "userId: " << userId;
}

void AuthForm::sendAuthReq()
{
    /// определяем тип запроса в зав-ти от кнопки
    RequestType reqType;
    QString reqTypeStr = qobject_cast<QPushButton*>(sender())->text();

    if (reqTypeStr == "Login") reqType = RequestType::LOGIN;
    else if (reqTypeStr == "Register") reqType = RequestType::REGISTER;
    else {
        qDebug() << "Неизвестный тип запроса, скорее всего ошибка каста кнопки";
        return;
    }

    QString login = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    QByteArray data = Serializer::serializeAuthReq(login, password);
    QByteArray packet = PacketBuilder::createPacketToSend(data, reqType);

    m_socket->write(packet);
}

void AuthForm::sendGetReq(const int userId)
{
    QByteArray serializedData = Serializer::serializeGetReq(userId);
    QByteArray packet = PacketBuilder::createPacketToSend(serializedData, RequestType::GET_CHATS);

    m_socket->write(packet);
}
