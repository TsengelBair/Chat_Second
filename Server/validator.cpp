#include <QDebug>
#include <QDataStream>

#include "validator.h"

bool Validator::validatePacket(const QByteArray &packet)
{
    /// Извлекаем первые 5 байт (4 - размер, 1 - CRC)
    QByteArray header = packet.left(headerSize - 1);
    QDataStream ds(&header, QIODevice::ReadOnly);

    qint32 dataSizeFromPacket;
    qint8 crcFromPacket;

    ds >> dataSizeFromPacket >> crcFromPacket;

    /// Получаем данные (всё, что после 6 байт)
    QByteArray packetData = packet.mid(headerSize);
    if (packetData.size() != dataSizeFromPacket) {
        qDebug() << "Указанный в заголовке размер данных не совпал с фактическим размером";
        return false;
    }

    /// Проверяем контрольную сумму
    uint8_t crc = calcCrc(packetData);
    if (static_cast<uint8_t>(crcFromPacket) != crc) {
        qDebug() << "CRC mismatch!";
        return false;
    }

    return true;
}

uint8_t Validator::calcCrc(const QByteArray &packetData)
{
    uint8_t crc = 0;
    for (const char c : packetData) {
        crc ^= static_cast<uint8_t>(c);
    }
    return crc;
}

RequestType Validator::getRequestType(const QByteArray &packet)
{
    return static_cast<RequestType>(packet.at(5));  /// 6-й байт (индекс 5)
}
