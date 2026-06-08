#pragma once

#include <QHttpServer>
#include <QHash>

class LocalServer
{
public:
    static bool start();
    static QHash<QString, int> sessions;

private:
    static QHttpServer server;
};