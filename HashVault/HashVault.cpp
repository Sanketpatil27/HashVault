#include "HashVault.h"
#include "Database/DBManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <qmessagebox.h>
#include <QInputDialog>
#include "server/LocalServer.h"
#include <QApplication>

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