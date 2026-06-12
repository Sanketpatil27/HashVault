#include "HashVault.h"

#include <QMessageBox>
#include <QTimer>

void HashVault::setupSettingsConnections()
{
    autoLockTimer = new QTimer(this);

    autoLockTimer->setSingleShot(true);

    connect(ui.autoLockCheckBox, &QCheckBox::toggled, this, &HashVault::handleAutoLock);
    connect(autoLockTimer, &QTimer::timeout, this, &HashVault::autoLockVault);
}



void HashVault::handleAutoLock(bool enabled)
{
    if (enabled)
        autoLockTimer->start(60000);
    else
        autoLockTimer->stop();
}

void HashVault::autoLockVault()
{
    QMessageBox::information(this, "Vault Locked", "Session expired due to inactivity");
    logoutUser();
}