#include <QString>
#include <QDebug>

#include "serializer.h"
#include "dbhandler.h"
#include "../ProtoFiles/IAuthRequest.pb.h"
#include "../ProtoFiles/IAuthResponse.pb.h"
#include "../ProtoFiles/IGetRequest.pb.h"
#include "../ProtoFiles/IGetResponse.pb.h"
#include "../ProtoFiles/ISearchRequest.pb.h"
#include "../ProtoFiles/ISearchResponse.pb.h"


QByteArray Serializer::serializeAuthResponse(IAuthResponse *response)
{
    std::string serialized_data;
    if (!response->SerializeToString(&serialized_data)){
        qDebug() << "Error while serialize Auth Response";
    }

    QByteArray data(serialized_data.c_str(), serialized_data.size());
    return data;
}

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

GetRequestBody Serializer::deserializeGetRequest(const QByteArray &packetData)
{
    IGetRequest getReq;
    if (!getReq.ParseFromString(packetData.toStdString())) {
        qDebug() << "Error while deserialize auth request";
        return GetRequestBody();
    }

    GetRequestBody getRequest;
    getRequest.userId = getReq.userid();

    /// проверяем, т.к. в прото файле поля указаны как опциональные и м.б. не заданы
    if (getReq.has_chats_limit()) {
        getRequest.chatsLimit = getReq.chats_limit();
    }

    if (getReq.has_messages_in_chat_limit()) {
        getRequest.messagesInChatLimit = getReq.messages_in_chat_limit();
    }

    return getRequest;
}

QString Serializer::deserializeFindUserRequest(const QByteArray &packetData)
{
    ISearchRequest searchReq;
    if (!searchReq.ParseFromString(packetData.toStdString())) {
        qDebug() << "Error while deserialize auth request";
        return QString();
    }

    QString loginToFind = QString::fromStdString(searchReq.login());
    return loginToFind;
}

QByteArray Serializer::serializeGetResponse(QVector<Chat> &chats, bool isEmpty)
{
    IGetResponse response;
    response.set_has_more_chats(false); /// пока что пусть будет по умолчанию false

    if (!isEmpty) {
        for (const Chat &chat : chats) {
            IChat *protoChat = response.add_chats();
            protoChat->set_chat_id(chat.chatId);
            protoChat->set_interlocutor_name(chat.interlocutorName.toStdString());

            for (const Message &message : chat.messages) {
                IChatMessage *protoMessage = protoChat->add_messages();
                protoMessage->set_message_id(message.id);
                protoMessage->set_sender_id(message.senderId);
                protoMessage->set_sender_name(message.senderName.toStdString());
                protoMessage->set_message_content(message.content.toStdString());
                protoMessage->set_message_timestamp(message.timestamp.toString(Qt::ISODate).toStdString());
            }
        }
    }

    response.set_is_empty(isEmpty);

    /// Сериализация ответа
    std::string serializedData;
    if (!response.SerializeToString(&serializedData)) {
        qDebug() << "Error while serialize Get Response";
        return QByteArray();
    }

    /// Отправка сериализованного ответа клиенту
    QByteArray data(serializedData.c_str(), serializedData.size());
    return data;
}

QByteArray Serializer::serializeFindedUsersResponse(QList<QString> foundUsers)
{
    ISearchResponse response;

    for (const QString& user : foundUsers) {
        response.add_find_logins(user.toStdString());
    }

    response.set_is_empty(foundUsers.empty());

    std::string serializedStr = response.SerializeAsString();
    return QByteArray(serializedStr.data(), static_cast<int>(serializedStr.size()));
}

