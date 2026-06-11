#include "CryptoManager.h"


#include <openssl/evp.h>
#include <openssl/rand.h>

#include <QByteArray>

QByteArray CryptoManager::currentKey;

void CryptoManager::setCurrentKey(const QByteArray& key)
{
    currentKey = key;
}

QString CryptoManager::encrypt(const QString& plainText)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
        return "";

    QByteArray input = plainText.toUtf8();

    QByteArray encrypted;
    encrypted.resize(input.size() + EVP_MAX_BLOCK_LENGTH);

    int outLen1 = 0;
    int outLen2 = 0;

    QByteArray key = currentKey;
    QByteArray iv(16, 0);

    RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()),16);

    EVP_EncryptInit_ex(
        ctx,
        EVP_aes_256_cbc(),
        nullptr,
        reinterpret_cast<const unsigned char*>(key.constData()),
        reinterpret_cast<const unsigned char*>(iv.constData())
    );

    EVP_EncryptUpdate(
        ctx,
        reinterpret_cast<unsigned char*>(encrypted.data()),
        &outLen1,
        reinterpret_cast<const unsigned char*>(input.constData()),
        input.size()
    );

    EVP_EncryptFinal_ex(
        ctx,
        reinterpret_cast<unsigned char*>(encrypted.data()) + outLen1,
        &outLen2
    );

    EVP_CIPHER_CTX_free(ctx);

    encrypted.resize(outLen1 + outLen2);

    QByteArray result = iv + encrypted;

    return result.toBase64();
}

QString CryptoManager::decrypt(const QString& cipherText)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
        return "";

    QByteArray combined =
        QByteArray::fromBase64(
            cipherText.toUtf8()
        );

    if (combined.size() < 16)
    {
        return "";
    }

    QByteArray iv = combined.left(16);

    QByteArray input = combined.mid(16);

    QByteArray decrypted;
    decrypted.resize(input.size());

    int outLen1 = 0;
    int outLen2 = 0;

    QByteArray key = currentKey;

    EVP_DecryptInit_ex(
        ctx,
        EVP_aes_256_cbc(),
        nullptr,
        reinterpret_cast<const unsigned char*>(key.constData()),
        reinterpret_cast<const unsigned char*>(iv.constData())
    );

    EVP_DecryptUpdate(
        ctx,
        reinterpret_cast<unsigned char*>(decrypted.data()),
        &outLen1,
        reinterpret_cast<const unsigned char*>(input.constData()),
        input.size()
    );

    EVP_DecryptFinal_ex(
        ctx,
        reinterpret_cast<unsigned char*>(decrypted.data()) + outLen1,
        &outLen2
    );

    EVP_CIPHER_CTX_free(ctx);

    decrypted.resize(outLen1 + outLen2);

    return QString::fromUtf8(decrypted);
}