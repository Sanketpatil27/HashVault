#pragma once

#include <QString>

class CryptoManager
{
public:
    static QString encrypt(const QString& plainText);
    static QString decrypt(const QString& cipherText);

private:
    static QByteArray getEncryptionKey();
};