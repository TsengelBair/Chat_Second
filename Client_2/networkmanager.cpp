#include "networkmanager.h"
#include "validator.h"

NetworkManager::NetworkManager(QObject *parent) :QObject(parent)
{
    m_socket.reset(new QTcpSocket()); /// по идее можно без this
    connect(m_socket.data(), &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
}

NetworkManager::~NetworkManager()
{
    m_socket->close(); /// по идее необязательно указывать, но пусть будет
}

bool NetworkManager::connectToServer(const QString &host, quint16 port)
{
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(3000); /// это хз зачем
}

void NetworkManager::onReadyRead()
{
    while (m_socket->bytesAvailable()) {
        QByteArray packet = m_socket->readAll();
        if (packet.size() < Validator::headerSize) {
            qDebug() << "Пришло меньше 6 байт";
            return;
        }

        proccessPacket(packet);
    }
}

void NetworkManager::proccessPacket(const QByteArray &packet)
{
    if (!Validator::validatePacket(packet)) {
        qDebug() << "Ошибка валидации пакета";
        return;
    }

    ResponseType responseType = Validator::getResponseType(packet);
    QByteArray data = packet.mid(Validator::headerSize);

    switch (responseType) {
        case ResponseType::RESPONSE_LOGIN:
        case ResponseType::RESPONSE_REGISTER:
            emit signalAuthResponseReceived(data, responseType);
            break;
        case ResponseType::RESPONSE_GET_DEFAULT_DATA:
            emit signalGetDefaultDataResponseReceived(data);
            break;
        case ResponseType::RESPONSE_FIND_USERS:
            emit signalGetFoundedUsersResponseReceived(data);
            break;

        default:
            qDebug() << "Unknown response type:" << responseType;
            return;
    }
}
