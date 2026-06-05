#pragma once

#include <QObject>
#include <QHttpServer>
#include "../database/DBManager.h"

class LocalServer : public QObject
{
    Q_OBJECT

public:
    explicit LocalServer(DBManager* db, QObject* parent = nullptr);
    void start(int port = 45678);

private:
    QHttpServer    server_;
    DBManager* db_;

    // helpers
    QHttpServerResponse  makeJson(const QJsonDocument& doc);
    QHttpServerResponse  makeError(const QString& msg, int status = 400);
};