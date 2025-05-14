#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "dbhandler.h"

DbHandler* DbHandler::instance = nullptr;

DbHandler::DbHandler()
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("chat");
    db.setUserName("postgres");
    openDb();
}

DbHandler::~DbHandler()
{
    if (db.isOpen()) {
        db.close();
    }
}

void DbHandler::openDb()
{
    if (db.open()) {
        qDebug() << "Success connection to db";
        bool ok = createDefaultTables();
        if (ok) {
            qDebug() << "Default tables created";
        }
    } else {
        qDebug() << "Connection to db failed " << db.lastError().text();
    }
}

/// метод создания таблиц по умолчанию
bool DbHandler::createDefaultTables()
{
    QSqlQuery query;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY,
            login TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL
        );
    )")) {
        qDebug() << "Ошибка при создании таблицы users:" << query.lastError();
        return false;
    }

    /// private_chats - чаты 1 на 1, в дальнейшем планируется добавить групповые чаты
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS private_chats (
            id SERIAL PRIMARY KEY,
            user1_id INTEGER NOT NULL,
            user2_id INTEGER NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            UNIQUE (user1_id, user2_id),
            FOREIGN KEY (user1_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (user2_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )")) {
        qDebug() << "Ошибка при создании таблицы private_chats:" << query.lastError();
        return false;
    }

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id SERIAL PRIMARY KEY,
            chat_id INTEGER NOT NULL,
            sender_id INTEGER NOT NULL,
            content TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (chat_id) REFERENCES private_chats(id) ON DELETE CASCADE,
            FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )")) {
        qDebug() << "Ошибка при создании таблицы messages:" << query.lastError();
        return false;
    }

    return true;
}

DbHandler *DbHandler::getInstance()
{
    if (!instance) {
        instance = new DbHandler();
    }

    return instance;
}

bool DbHandler::userExist(const QString &login) const
{
    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса существования пользователя" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count > 0;
    }

    return false;
}

bool DbHandler::checkPassword(const QString &login, const QString &password) const
{
    QSqlQuery query;

    query.prepare("SELECT password FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса проверки пароля" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        QString storedPassword = query.value(0).toString();
        return storedPassword == password;
    }

    return false;
}

int DbHandler::addUser(const QString &login, const QString &password)
{
    QSqlQuery query;

    query.prepare("INSERT INTO users (login, password) VALUES (:login, :password) RETURNING id");
    query.bindValue(":login", login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса вставки нового пользователя:" << query.lastError().text();
        return -1;
    }

    /// Читаем возвращенный ID
    if (query.next()) {
        bool ok;
        int userId = query.value(0).toInt(&ok);
        if (ok) {
            return userId;
        }
    }

    qDebug() << "Не удалось получить ID нового пользователя";
    return -1;
}

int DbHandler::getUserId(const QString &login) const
{
    QSqlQuery query;

    query.prepare("SELECT id FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса получения id пользователя" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        int id = query.value(0).toInt();
        return id;
    }

    return -1;
}

QVector<Chat> DbHandler::loadUserChats(int userId, int limit)
{
    QVector<Chat> chats;

    QSqlQuery query;
    query.prepare(R"(
        SELECT
            pc.id AS chat_id,
            CASE
                WHEN pc.user1_id = :user_id THEN u2.login
                ELSE u1.login
            END AS interlocutor_name
        FROM
            private_chats pc
        JOIN users u1 ON pc.user1_id = u1.id
        JOIN users u2 ON pc.user2_id = u2.id
        WHERE
            pc.user1_id = :user_id OR pc.user2_id = :user_id
        LIMIT :limit
    )");

    query.bindValue(":user_id", userId);
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qDebug() << "Ошибка загрузки чатов:" << query.lastError().text();
        return chats;
    }

    while (query.next()) {
        Chat chat;
        chat.chatId = query.value("chat_id").toInt();
        chat.interlocutorName = query.value("interlocutor_name").toString();
        chats.append(chat);
    }

    return chats;
}


QVector<Message> DbHandler::loadChatMessages(int chatId, int limit)
{
    QVector<Message> messages;

    QSqlQuery query;
    query.prepare(R"(
        SELECT
            m.id,
            m.sender_id,
            u.login AS sender_name,
            m.content,
            m.created_at AS timestamp
        FROM
            messages m
        JOIN users u ON m.sender_id = u.id
        WHERE
            m.chat_id = :chat_id
        ORDER BY
            m.created_at DESC
        LIMIT :limit
    )");

    query.bindValue(":chat_id", chatId);
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qDebug() << "Ошибка загрузки сообщений:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        Message msg;
        msg.id = query.value("id").toInt();
        msg.senderId = query.value("sender_id").toInt();
        msg.senderName = query.value("sender_name").toString();
        msg.content = query.value("content").toString();
        msg.timestamp = query.value("timestamp").toDateTime();

        messages.append(msg);
    }

    return messages;
}

QList<QString> DbHandler::findUsersWithSimilarLogin(const QString &prefix)
{
    QList<QString> users;

    QSqlQuery query;
    query.prepare(R"(
        SELECT login FROM users
        WHERE login LIKE :pattern || '%'
        ORDER BY login
        LIMIT 50
    )");

    query.bindValue(":pattern", prefix);

    if (!query.exec()) {
        qWarning() << "Search query failed:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        users.append(query.value("login").toString());
    }

    return users;
}
