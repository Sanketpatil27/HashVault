#pragma once

#include <QHttpServer>
#include <QHash>

class LocalServer
{
public:
    static bool start();

private:
    static QHttpServer server;
};