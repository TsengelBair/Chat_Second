#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QByteArray>
#include <QPair>

class IAuthResponse;
struct Chat;

struct GetRequestBody
{
    int userId;
    int chatsLimit = 20; /// если в запросе не указано явно кол-во чатов и сообщений, по умолчанию max значение это 20
    int messagesInChatLimit = 20;
};

class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    static QByteArray serializeAuthResponse(IAuthResponse* response);
    static QByteArray serializeGetResponse(QVector<Chat> &chats, bool isEmpty);
    static QByteArray serializeFindedUsersResponse(QList<QString> foundUsers);

    static QPair<QString, QString> deserializeAuthRequest(const QByteArray &packetData);
    static GetRequestBody deserializeGetRequest(const QByteArray &packetData);
    static QString deserializeFindUserRequest(const QByteArray &packetData);
};

#endif // SERIALIZER_H
