#include "LocalServer.h"
#include <jwt-cpp/jwt.h>
#include "../security/JwtManager.h"
#include "../security/CryptoManager.h"
#include <qmessagebox.h>

#include <QSqlQuery>
#include <QSqlError>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QHttpServerRequest>
#include <QTcpServer>
#include <QHostAddress>
#include <QUuid>
#include <QCryptographicHash>

QHttpServer LocalServer::server;


bool LocalServer::start()
{
    // =====================================
    // HEALTH CHECK
    // =====================================
   
    server.route(
        "/ping",
        []()
        {
            return QString("OK");
        }
    );

    // =====================================
    // LOGIN
    // =====================================

    server.route(
        "/login/<arg>/<arg>",
        [](const QString& username,
            const QString& password)
        {
            QSqlQuery query;

            query.prepare(
                "SELECT id, password, salt "
                "FROM users "
                "WHERE username = ?"
            );

            query.addBindValue(username);

            if (!query.exec())
            {
                return QByteArray(
                    R"({"success":false})"
                );
            }

            if (!query.next())
            {
                return QByteArray(
                    R"({"success":false})"
                );
            }

            QString storedHash = query.value("password").toString();
            QString salt = query.value("salt").toString();

            QByteArray enteredHash = 
                QCryptographicHash::hash(
                    (password + salt).toUtf8(),
                    QCryptographicHash::Sha256
                ).toHex();

            if (storedHash != enteredHash)
            {
                return QByteArray(
                    R"({"success":false})"
                );
            }

            int userId = query.value("id").toInt();

            QString token = JwtManager::generateToken(userId, username);

            QJsonObject response;

            response["success"] = true;
            response["token"] = token;

            return QJsonDocument(response)
                .toJson(QJsonDocument::Compact);
        }
    );


    // =====================================
    // GET ALL PASSWORDS
    // =====================================

    server.route(
        "/passwords/<arg>",
        [](const QString& token)
        {
            int userId = JwtManager::validateToken(token);

            if (userId == -1)
            {
                return QByteArray("[]");
            }

            QSqlQuery query;

            query.prepare(
                "SELECT id, website, username, password, category, notes "
                "FROM passwords "
                "WHERE user_id = ? "
                "ORDER BY id"
            );
            
            query.addBindValue(userId);

            if (!query.exec())
            {
                return QByteArray("[]");
            }

            QJsonArray passwords;

            while (query.next())
            {
                QJsonObject password;

                password["id"] =
                    query.value("id").toInt();

                password["website"] =
                    query.value("website").toString();

                password["username"] =
                    query.value("username").toString();

                password["password"] =
                    CryptoManager::decrypt(
                        query.value("password").toString()
                    );

                password["category"] =
                    query.value("category").toString();

                password["notes"] =
                    query.value("notes").toString();

                passwords.append(password);
            }

            return QJsonDocument(passwords)
                .toJson(QJsonDocument::Compact);
        }
    );


    // =====================================
    // GET PASSWORD FOR WEBSITE
    // Example:
    // /credential/token/github.com
    // =====================================

    server.route(
        "/credential/<arg>/<arg>",
        [](const QString& token,
            const QString& website)
        {
            int userId = JwtManager::validateToken(token);

            if (userId == -1)
            {
                return QByteArray("[]");
            }

            QSqlQuery query;

            query.prepare(
                "SELECT id, website, username, password, category, notes "
                "FROM passwords "
                "WHERE website = ? "
                "AND user_id = ? "
                "LIMIT 1"
            );

            query.addBindValue(website);
            query.addBindValue(userId);

            if (!query.exec())
            {
                return QByteArray("{}");
            }

            if (!query.next())
            {
                return QByteArray("{}");
            }

            QJsonObject credential;

            credential["id"] =
                query.value("id").toInt();

            credential["website"] =
                query.value("website").toString();

            credential["username"] =
                query.value("username").toString();

            credential["password"] =
                CryptoManager::decrypt(
                    query.value("password").toString()
                );

            credential["category"] =
                query.value("category").toString();

            credential["notes"] =
                query.value("notes").toString();

            return QJsonDocument(credential)
                .toJson(QJsonDocument::Compact);
        }
    );

    //=======================
    // Add Password Route
    //=======================
    server.route(
        "/addPassword/<arg>/<arg>/<arg>/<arg>/<arg>/<arg>",
        [](const QString& token,
            const QString& website,
            const QString& username,
            const QString& password,
            const QString& category,
            const QString& notes)
        {
            int userId =
                JwtManager::validateToken(token);

            if (userId == -1)
            {
                return QByteArray(
                    R"({"success":false})"
                );
            }

            QSqlQuery query;

            query.prepare(
                "INSERT INTO passwords "
                "(website, username, password, category, notes, user_id) "
                "VALUES (?, ?, ?, ?, ?, ?)"
            );

            query.addBindValue(website);
            query.addBindValue(username);

            query.addBindValue(
                CryptoManager::encrypt(password)
            );

            query.addBindValue(category);
            query.addBindValue(notes);
            query.addBindValue(userId);

            bool success = query.exec();

            QJsonObject response;

            response["success"] = success;

            return QJsonDocument(response)
                .toJson(QJsonDocument::Compact);
        }
    );

    //=======================
    // Update Password Route
    //=======================
    server.route(
        "/updatePassword/<arg>/<arg>/<arg>/<arg>/<arg>/<arg>/<arg>",
        [](const QString& token,
            const QString& id,
            const QString& website,
            const QString& username,
            const QString& password,
            const QString& category,
            const QString& notes)
        {
            int userId =
                JwtManager::validateToken(token);

            if (userId == -1)
            {
                return QByteArray(
                    R"({"success":false})"
                );
            }

            QSqlQuery query;

            query.prepare(
                "UPDATE passwords "
                "SET website=?, "
                "username=?, "
                "password=?, "
                "category=?, "
                "notes=? "
                "WHERE id=? "
                "AND user_id=?"
            );

            query.addBindValue(website);
            query.addBindValue(username);

            query.addBindValue(
                CryptoManager::encrypt(password)
            );

            query.addBindValue(category);
            query.addBindValue(notes);

            query.addBindValue(
                id.toInt()
            );

            query.addBindValue(userId);

            bool success =
                query.exec();

            QJsonObject response;

            response["success"] =
                success;

            return QJsonDocument(response)
                .toJson(QJsonDocument::Compact);
        }
    );

    //=======================
    // Delete Password Route
    //=======================
    server.route(
        "/deletePassword/<arg>/<arg>",
        [](const QString& token,
            const QString& id)
        {
            int userId =
                JwtManager::validateToken(token);

            if (userId == -1)
            {
                return QByteArray(
                    R"({"success":false})"
                );
            }

            QSqlQuery query;

            query.prepare(
                "DELETE FROM passwords "
                "WHERE id=? "
                "AND user_id=?"
            );

            query.addBindValue(id.toInt());

            query.addBindValue(userId);

            bool success = query.exec();

            QJsonObject response;

            response["success"] = success;

            return QJsonDocument(response)
                .toJson(QJsonDocument::Compact);
        }
    );

    //=======================
    // New Category Route
    //=======================
    server.route(
        "/categories/<arg>",
        [](const QString& token)
        {
            int userId =
                JwtManager::validateToken(token);

            if (userId == -1)
            {
                return QByteArray("[]");
            }

            QSqlQuery query;

            query.prepare(
                "SELECT DISTINCT category "
                "FROM passwords "
                "WHERE user_id = ? "
                "AND category IS NOT NULL "
                "AND category <> '' "
                "ORDER BY category"
            );

            query.addBindValue(userId);

            if (!query.exec())
            {
                return QByteArray("[]");
            }

            QJsonArray categories;

            while (query.next())
            {
                categories.append(
                    query.value(0).toString()
                );
            }

            return QJsonDocument(categories)
                .toJson(QJsonDocument::Compact);
        }
    );

    // =====================================
    // START SERVER
    // =====================================
    QTcpServer* tcpServer = new QTcpServer();

    if (!tcpServer->listen(QHostAddress::Any, 8080))
    {
        QMessageBox::critical(
            nullptr,
            "Server Error",
            "Failed to start local server on port 8080"
        );
        delete tcpServer;
        return false;
    }

    server.bind(tcpServer);

    //QMessageBox::information(
    //    nullptr,
    //    "Server Started",
    //    "HashVault local server started on port 8080"
    //);

    return true;
}