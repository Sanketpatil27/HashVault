#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HashVault.h"

class HashVault : public QMainWindow
{
    Q_OBJECT

public:
    HashVault(QWidget *parent = nullptr);
    ~HashVault();


private slots:
    void handleLogin();
    void openSettings();
    //void backToDashboard();
    void openAddPasswordPage();
    void backToDashboardFromAddPage();
    void backToDashboardFromSettings();
    void openRegisterPage();
    void openLoginPage();

private:
    Ui::HashVaultClass ui;
};

