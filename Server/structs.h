#ifndef STRUCTS_H
#define STRUCTS_H

#include <QString>
#include <QDateTime>

/// объект сообщения
struct Message
{
    int id;
    int senderId;
    QString senderName;
    QString content;
    QDateTime timestamp;
};

/// объект чата с id и всеми сообщениями
struct Chat
{
    int chatId;
    QString interlocutorName; /// имя собеседника - название чата
    QVector<Message> messages;
};

#endif // STRUCTS_H
