#pragma once

#include <QString>

class JwtManager
{
public:
    static QString generateToken(
        int userId,
        const QString& username
    );

    static int validateToken(
        const QString& token
    );
};