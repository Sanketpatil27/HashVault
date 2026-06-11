#pragma once

#include <QString>
#include <QByteArray>

class CryptoManager
{
public:
    static void setCurrentKey(const QByteArray& key);

    static QString encrypt(const QString& plainText);
    static QString decrypt(const QString& cipherText);

private:
    static QByteArray currentKey;
};