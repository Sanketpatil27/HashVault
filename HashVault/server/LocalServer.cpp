//#include "LocalServer.h"
//#include <QHttpServerRequest>
//#include <QHttpServerResponse>
//#include <QJsonDocument>
//#include <QJsonObject>
//#include <QJsonArray>
//#include <QDebug>
//
//LocalServer::LocalServer(DBManager* db, QObject* parent)
//    : QObject(parent), db_(db)
//{
//}
//
//void LocalServer::start(int port)
//{
//    // ── Allow requests from the browser extension ─────────────────
//    auto addCors = [](QHttpServerResponse res) {
//        res.setHeader("Access-Control-Allow-Origin", "*");
//        res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
//        res.setHeader("Access-Control-Allow-Headers", "Content-Type");
//        return res;
//        };
//
//    // ── GET /passwords?userId=X&host=github.com ───────────────────
//    // Returns passwords whose website matches the current page host
//    server_.route("/passwords", QHttpServerRequest::Method::Get,
//        [this, addCors](const QHttpServerRequest& req) {
//
//            auto params = req.query();
//            int  userId = params.queryItemValue("userId").toInt();
//            QString host = params.queryItemValue("host");
//
//            if (userId <= 0)
//                return addCors(makeError("userId required"));
//
//            auto entries = db_->searchPasswords(userId, host);
//
//            QJsonArray arr;
//            for (const auto& e : entries) {
//                QJsonObject obj;
//                obj["id"] = e.id;
//                obj["website"] = e.website;
//                obj["username"] = e.username;
//                // Never send plain-text password to the extension
//                // The extension will ask Qt app to fill the field directly
//                arr.append(obj);
//            }
//
//            return addCors(makeJson(QJsonDocument(arr)));
//        });
//
//    // ── POST /autofill  { userId, passwordId } ────────────────────
//    // Returns the actual password for autofill — only for local requests
//    server_.route("/autofill", QHttpServerRequest::Method::Post,
//        [this, addCors](const QHttpServerRequest& req) {
//
//            auto body = QJsonDocument::fromJson(req.body()).object();
//            int userId = body["userId"].toInt();
//            int passwordId = body["passwordId"].toInt();
//
//            if (userId <= 0 || passwordId <= 0)
//                return addCors(makeError("userId and passwordId required"));
//
//            auto entry = db_->getPasswordById(passwordId);
//
//            if (entry.id == -1)
//                return addCors(makeError("Not found", 404));
//
//            QJsonObject res;
//            res["username"] = entry.username;
//            res["password"] = entry.password;  // only sent on explicit user action
//
//            return addCors(makeJson(QJsonDocument(res)));
//        });
//
//    // ── OPTIONS (pre-flight CORS) ─────────────────────────────────
//    server_.route("/passwords", QHttpServerRequest::Method::Options,
//        [addCors](const QHttpServerRequest&) {
//            return addCors(QHttpServerResponse(QHttpServerResponse::StatusCode::Ok));
//        });
//    server_.route("/autofill", QHttpServerRequest::Method::Options,
//        [addCors](const QHttpServerRequest&) {
//            return addCors(QHttpServerResponse(QHttpServerResponse::StatusCode::Ok));
//        });
//
//    if (server_.listen(QHostAddress::LocalHost, port))
//        qDebug() << "HashVault local server running on port" << port;
//    else
//        qDebug() << "Failed to start local server";
//}
//
//QHttpServerResponse LocalServer::makeJson(const QJsonDocument& doc)
//{
//    return QHttpServerResponse("application/json", doc.toJson());
//}
//
//QHttpServerResponse LocalServer::makeError(const QString& msg, int status)
//{
//    QJsonObject obj;
//    obj["error"] = msg;
//    QHttpServerResponse res("application/json", QJsonDocument(obj).toJson());
//    res.setStatusCode(static_cast<QHttpServerResponse::StatusCode>(status));
//    return res;
//}