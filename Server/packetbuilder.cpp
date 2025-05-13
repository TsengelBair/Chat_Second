#include <QDataStream>

#include "packetbuilder.h"
#include "validator.h"

QByteArray PacketBuilder::createPacketToSend(const QByteArray &data, const ResponseType &responseType)
{
    QByteArray packet;

    int packetSize = data.size();

    /// первые 4 байта размер данных
    QDataStream out(&packet, QIODevice::WriteOnly);
    out << qint32(packetSize);

    /// пятый байт контрольная сумма
    uint8_t crc = Validator::calcCrc(data);
    packet.append(static_cast<char>(crc));

    /// шестой тип запроса
    packet.append(static_cast<uint8_t>(responseType));

    /// сами данные
    packet.append(data);
    return packet;
}
