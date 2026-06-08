#include "DBManager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QSettings>
#include <QFile>

QString DBManager::m_lastError = "";

bool DBManager::createDatabaseIfNotExists()
{
    QSettings settings("config/database.ini", QSettings::IniFormat);

    QString databaseName =
        settings.value("Database/Database").toString();

    QSqlDatabase db =
        QSqlDatabase::addDatabase("QPSQL", "db_creator");

    db.setHostName(settings.value("Database/Host").toString());
    db.setDatabaseName("postgres");
    db.setUserName(settings.value("Database/Username").toString());
    db.setPassword(settings.value("Database/Password").toString());
    db.setPort(settings.value("Database/Port").toInt());

    if (!db.open())
    {
        m_lastError = db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    if (!query.exec(
        QString("SELECT 1 FROM pg_database WHERE datname='%1'")
        .arg(databaseName)))
    {
        m_lastError = query.lastError().text();
        return false;
    }

    if (!query.next())
    {
        if (!query.exec(
            QString("CREATE DATABASE \"%1\"")
            .arg(databaseName)))
        {
            m_lastError = query.lastError().text();
            return false;
        }
    }

    db.close();

    return true;
}

bool DBManager::connectDatabase()
{
    if (!QFile::exists("config/database.ini"))
    {
        m_lastError = "config/database.ini not found";
        return false;
    }

    if (!createDatabaseIfNotExists())
        return false;

    QSettings settings("config/database.ini", QSettings::IniFormat);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    db.setHostName(settings.value("Database/Host").toString());
    db.setDatabaseName(settings.value("Database/Database").toString());
    db.setUserName(settings.value("Database/Username").toString());
    db.setPassword(settings.value("Database/Password").toString());
    db.setPort(settings.value("Database/Port").toInt());

    if (!db.open())
    {
        m_lastError = db.lastError().text();
        return false;
    }

    return createTables();
}

bool DBManager::createTables()
{
    QSqlQuery query;

    bool success =
        query.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id SERIAL PRIMARY KEY,"
            "fullname VARCHAR(100) NOT NULL,"
            "username VARCHAR(50) UNIQUE NOT NULL,"
            "email VARCHAR(100) UNIQUE NOT NULL,"
            "password VARCHAR(255) NOT NULL"
            ");"
        );

    if (!success)
    {
        m_lastError = query.lastError().text();
        return false;
    }

    success =
        query.exec(
            "CREATE TABLE IF NOT EXISTS passwords ("
            "id SERIAL PRIMARY KEY,"
            "website VARCHAR(100) NOT NULL,"
            "username VARCHAR(100) NOT NULL,"
            "password VARCHAR(255) NOT NULL,"
            "category VARCHAR(50),"
            "user_id INTEGER NOT NULL,"
            "notes TEXT,"
            "CONSTRAINT fk_user "
            "FOREIGN KEY(user_id) "
            "REFERENCES users(id) "
            "ON DELETE CASCADE"
            ");"
        );

    if (!success)
    {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

QString DBManager::lastError()
{
    return m_lastError;
}