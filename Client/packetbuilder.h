#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include <QByteArray>
#include "types.h"

class PacketBuilder
{
public:
    /// статик класс
    PacketBuilder() = delete;
    PacketBuilder(const PacketBuilder&) = delete;
    PacketBuilder& operator=(const PacketBuilder&) = delete;

    static QByteArray createPacketToSend(const QByteArray &data, const RequestType &requestType);
};

#endif // PACKETBUILDER_H
