#pragma once
#include <QString>

class DBManager
{
public:
    static bool connectDatabase();
    static bool createDatabaseIfNotExists();
    static QString lastError();

private:
    static bool createTables();
    static QString m_lastError;
};