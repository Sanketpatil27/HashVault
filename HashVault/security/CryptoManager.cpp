#include "CryptoManager.h"

#include <openssl/evp.h>

#include <QByteArray>

namespace
{
    const QByteArray AES_KEY =
        "12345678901234567890123456789012";

    const QByteArray AES_IV =
        "1234567890123456";
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

    EVP_EncryptInit_ex(
        ctx,
        EVP_aes_256_cbc(),
        nullptr,
        reinterpret_cast<const unsigned char*>(AES_KEY.constData()),
        reinterpret_cast<const unsigned char*>(AES_IV.constData())
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

    return encrypted.toBase64();
}

QString CryptoManager::decrypt(const QString& cipherText)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
        return "";

    QByteArray input =
        QByteArray::fromBase64(
            cipherText.toUtf8()
        );

    QByteArray decrypted;
    decrypted.resize(input.size());

    int outLen1 = 0;
    int outLen2 = 0;

    EVP_DecryptInit_ex(
        ctx,
        EVP_aes_256_cbc(),
        nullptr,
        reinterpret_cast<const unsigned char*>(AES_KEY.constData()),
        reinterpret_cast<const unsigned char*>(AES_IV.constData())
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