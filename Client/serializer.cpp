#include <QDebug>

#include "serializer.h"
#include "../ProtoFiles/IAuthRequest.pb.h"
#include "../ProtoFiles/IAuthResponse.pb.h"
#include "../ProtoFiles/IGetRequest.pb.h"

QPair<int, int> Serializer::deserializeAuthResponse(const QByteArray &data)
{
    IAuthResponse response;
    if (!response.ParseFromString(data.toStdString())) {
        qDebug() << "Error while deserialize auth response";
        return QPair<int, int>();
    }

    int serverResponseCode = response.code();
    int userId = response.userid();

    return qMakePair(serverResponseCode, userId);
}

QByteArray Serializer::serializeGetReq(const int userId)
{
    IGetRequest getReq;

    getReq.set_userid(userId);
    getReq.set_chats_limit(20); /// получаем максимум 20 чатов
    getReq.set_messages_in_chat_limit(20); /// максимум 20 сообщений в каждом чате

    std::string serialized_data;
    if (!getReq.SerializeToString(&serialized_data)){
        qDebug() << "Error while serialize get request";
    }

    QByteArray data(serialized_data.c_str(), serialized_data.size());
    return data;
}

QByteArray Serializer::serializeAuthReq(const QString &login, const QString &password)
{
    IAuthRequest request;

    request.set_login(login.toStdString());
    request.set_password(password.toStdString());

    std::string serialized_data;
    if (!request.SerializeToString(&serialized_data)){
        qDebug() << "Error while serialize";
    }

    QByteArray data(serialized_data.c_str(), serialized_data.size());
    return data;
}
