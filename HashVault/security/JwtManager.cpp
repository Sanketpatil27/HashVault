#include "JwtManager.h"

#include <jwt-cpp/jwt.h>

static const std::string SECRET =
"HashVaultSecretKey2026";

QString JwtManager::generateToken(
    int userId,
    const QString& username
)
{
    auto token =
        jwt::create()
        .set_type("JWT")
        .set_payload_claim(
            "userId",
            jwt::claim(
                std::to_string(userId)
            )
        )
        .set_payload_claim(
            "username",
            jwt::claim(
                username.toStdString()
            )
        )
        .sign(
            jwt::algorithm::hs256{
                SECRET
            }
        );

    return QString::fromStdString(
        token
    );
}

int JwtManager::validateToken(
    const QString& token
)
{
    try
    {
        auto decoded =
            jwt::decode(
                token.toStdString()
            );

        jwt::verify()
            .allow_algorithm(
                jwt::algorithm::hs256{
                    SECRET
                }
            )
            .verify(decoded);

        return std::stoi(
            decoded
            .get_payload_claim(
                "userId"
            )
            .as_string()
        );
    }
    catch (...)
    {
        return -1;
    }
}