#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QByteArray>
#include <QPair>

class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    static QByteArray serializeAuthReq(const QString &login, const QString &password);
    static QByteArray serializeGetReq(const int userId);

    static QPair<int, int> deserializeAuthResponse(const QByteArray &data);
};

#endif // SERIALIZER_H
