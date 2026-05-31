#include "DBManager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

bool DBManager::connectDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    db.setHostName("localhost");
    db.setDatabaseName("hashvault");
    db.setUserName("postgres");
    db.setPassword("root");
    db.setPort(5432);

    if (db.open())
    {
        //qDebug() << "Database Connected Successfully";
        return true;
    }

    //qDebug() << "Database Error:"
    //    << db.lastError().text();

    return false;
}