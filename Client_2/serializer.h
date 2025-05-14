#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QByteArray>

class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;

    static QByteArray serializeAuthReq(const QString &login, const QString &password);
};

#endif // SERIALIZER_H
