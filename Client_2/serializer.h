#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QByteArray>
#include <QPair>

#include "structs.h"

class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    static QByteArray serializeAuthReq(const QString &login, const QString &password);
    static QByteArray serializeGetReq(const int userId);
    static QByteArray serializeSearchUsersReq(const QString &loginToSearch);

    static QPair<int, int> deserializeAuthResponse(const QByteArray &data);
    static QVector<Chat> deserializeGetDefaultDataResponse(const QByteArray &data);
};

#endif // SERIALIZER_H
