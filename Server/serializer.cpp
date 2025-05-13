#include <QString>
#include <QDebug>

#include "serializer.h"
#include "dbhandler.h"
#include "../ProtoFiles/IAuthRequest.pb.h"
#include "../ProtoFiles/IAuthResponse.pb.h"
#include "../ProtoFiles/IGetRequest.pb.h"
#include "../ProtoFiles/IGetResponse.pb.h"
#include "../ProtoFiles/IGetResponseEmpty.pb.h"

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

QByteArray Serializer::serializeGetResponse(QVector<Chat> &chats)
{
    IGetResponse response;
    response.set_has_more_chats(false); /// пока что пусть будет по умолчанию false

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

QByteArray Serializer::serializeEmptyGetResponse()
{
    IGetResponseEmpty epmtyResponse;
    epmtyResponse.set_msg("Нет данных");

    std::string serializedData;
    if (!epmtyResponse.SerializeToString(&serializedData)) {
        qDebug() << "Error while serialize Empty Get Response";
        return QByteArray();
    }

    QByteArray data(serializedData.c_str(), serializedData.size());
    return data;
}
