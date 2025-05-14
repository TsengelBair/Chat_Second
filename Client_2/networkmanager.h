#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>

#include "types.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();

    bool connectToServer(const QString& host, quint16 port);
    void sendPacketToServer(const QByteArray &packet, const RequestType &reqType);

signals:
    /// param2 необходим для идентификации запроса, login || registr
    void signalAuthResponseReceived(const QByteArray &data, const ResponseType &responseType);
    /// после успешной авторизации подгружаем данные по умолчанию
    void signalGetDefaultDataResponseReceived(const QByteArray &data);
    /// при вводе логина в строку поиска, возвращаем список пользователей с похожим логином
    void signalGetFoundedUsersResponseReceived(const QByteArray &data);

private slots:
    void onReadyRead();

private:
    void proccessPacket(const QByteArray &packet);

private:
    QSharedPointer<QTcpSocket> m_socket;
};

#endif // NETWORKMANAGER_H
