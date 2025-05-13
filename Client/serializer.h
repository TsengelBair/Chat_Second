#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QByteArray>
#include <QPair>

class IAuthResponse;

class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    static QPair<int, int> deserializeAuthResponse(const QByteArray &data);
    static QByteArray serializeGetReq(const int userId);
    static QByteArray serializeAuthReq(const QString &login, const QString &password);

};

#endif // SERIALIZER_H
