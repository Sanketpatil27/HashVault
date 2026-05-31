#include "HashVault.h"
#include "Database/DBManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <qmessagebox.h>
#include <QInputDialog>
#include <QTimer>   

HashVault::HashVault(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    if (!DBManager::connectDatabase())
    {
        QMessageBox::critical(
            this,
            "Database Error",
            db.lastError().text()
        );
        return;
    }

    setupAuthConnections();
    setupPasswordConnections();
    setupSettingsConnections();
    setupHelperConnections();
}

HashVault::~HashVault()
{}
