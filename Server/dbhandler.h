#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <QSqlDatabase>
#include <QDateTime>

#include "structs.h"

class DbHandler
{
private:
    DbHandler();
    ~DbHandler();

    static DbHandler* instance;
    QSqlDatabase db;

private:
    void openDb();
    bool createDefaultTables();

public:
    static DbHandler* getInstance();
    bool userExist(const QString &login) const;
    bool checkPassword(const QString &login, const QString &password) const;
    int addUser(const QString &login, const QString &password);
    int getUserId(const QString &login) const;

    QVector<Chat> loadUserChats(int userId, int limit);
    QVector<Message> loadChatMessages(int chatId, int limit);
    QList<QString> findUsersWithSimilarLogin(const QString &loginToFind);
};


#endif // DBHANDLER_H
