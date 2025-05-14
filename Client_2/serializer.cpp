#include <QDebug>

#include "serializer.h"
#include "../ProtoFiles/IAuthRequest.pb.h"

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
