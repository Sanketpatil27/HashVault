#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HashVault.h"
#include <QSqlDatabase>

class HashVault : public QMainWindow
{
    Q_OBJECT

public:
    HashVault(QWidget *parent = nullptr);
    ~HashVault();


private slots:
    void openSettings();
    //void backToDashboard();
    void openAddPasswordPage();
    void backToDashboardFromAddPage();
    void backToDashboardFromSettings();
    void openRegisterPage();
    void openLoginPage();
    void registerUser();
    void loginUser();

private:
    Ui::HashVaultClass ui;
	QSqlDatabase db;        // create global database object to be used across the application
};

