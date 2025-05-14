#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

#include "types.h"

class IAuthResponse;

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QHostAddress address, qint16 port);
    ~Server();

private slots:
    void incomingConnection(qintptr socketDescriptor) override;
    void handleIncommingDataFromClient();
    void handleAuthRequest(const QByteArray &packetData, const RequestType &reqType, qintptr socketDescriptor);
    void handleGetRequest(const QByteArray &packetData, qintptr socketDescriptor);
    void handleFindUsersRequest(const QByteArray &packetData, qintptr socketDescriptor);
    void sendEmptyGetResponse(qintptr socketDescriptor);
    void sendToClient(const QByteArray &packet, qintptr socketDescriptor);

private:
    void handleRegistration(const QString &login, const QString &password, IAuthResponse *response);
    void handleLogin(const QString &login, const QString &password, IAuthResponse *response);

private:
    QMap<qintptr, QTcpSocket*> m_sockets;
};

#endif // SERVER_H
