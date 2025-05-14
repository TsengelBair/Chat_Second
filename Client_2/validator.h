#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QByteArray>
#include "types.h"

class Validator
{
public:
    Validator() = delete;
    Validator(const Validator&) = delete;
    Validator& operator=(const Validator&) = delete;

    static bool validatePacket(const QByteArray &packet);
    static uint8_t calcCrc(const QByteArray &packetData);
    static ResponseType getResponseType(const QByteArray &packet);

    static constexpr int headerSize = 6;
};

#endif // VALIDATOR_H
