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
    // Navigation
    void openSettings();
    void openAddPasswordPage();
    void backToDashboardFromAddPage();
    void backToDashboardFromSettings();
    void openRegisterPage();
    void openLoginPage();

    // Authentication 
    void registerUser();
    void loginUser();

    // Password CRUD Operations
    void addPassword();
    void loadPasswords(); 
    void deletePassword(int id);

private:
    Ui::HashVaultClass ui;
	QSqlDatabase db;                // create global database object to be used across the application
    int currentUserId = -1;         // store the logged-in user's id
};

