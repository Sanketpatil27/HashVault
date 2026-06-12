#include "HashVault.h"
#include "Database/DBManager.h"
#include "security/CryptoManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <qmessagebox.h>
#include <QInputDialog>
#include "server/LocalServer.h"
#include <QApplication>
#include <QSettings>

HashVault::HashVault(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    if (!DBManager::connectDatabase())
    {
        QMessageBox::critical(this, "Database Error", DBManager::lastError());
        return;
    }

    LocalServer::start();

    setupAuthConnections();
    setupPasswordConnections();
    setupSettingsConnections();
    setupHelperConnections();

    qApp->installEventFilter(this);

    QSettings settings("HashVault", "PasswordManager");

    if (settings.value("loggedIn", false).toBool())
    {
        currentUserId =
            settings.value("userId").toInt();

        currentUserDek =
            QByteArray::fromBase64(
                settings.value("userDek")
                .toByteArray()
            );

        if (currentUserDek.isEmpty())
        {
            settings.clear();
            ui.stackedWidget->setCurrentWidget(ui.loginPage);
            return;
        }

        CryptoManager::setCurrentKey(currentUserDek);

        loadPasswords();
        loadCategories();

        updatePasswordStatistics();

        ui.stackedWidget->setCurrentWidget(
            ui.dashboardPage
        );
    }
}

HashVault::~HashVault()
{}

bool HashVault::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj);

    if (autoLockTimer &&
        autoLockTimer->isActive())
    {
        switch (event->type())
        {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::KeyPress:
        case QEvent::Wheel:

            autoLockTimer->start(60000);
            break;

        default:
            break;
        }
    }

    return false;
}