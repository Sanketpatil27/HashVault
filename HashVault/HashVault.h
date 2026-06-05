#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HashVault.h"
#include <QSqlDatabase>
#include <QTimer>

class HashVault : public QMainWindow
{
    Q_OBJECT

public:
    HashVault(QWidget *parent = nullptr);
    ~HashVault();


private slots:
	// Navigation & UI Helpers
    void openSettings();
    void openAddPasswordPage();
    void backToDashboardFromSettings();
    void openRegisterPage();
    void openLoginPage();
    void cancelPasswordEditing();
    void togglePasswordVisibility(QLineEdit* field,bool checked);

    // Authentication 
    void registerUser();
    void loginUser();
    void logoutUser();

    // validation & hashing helpers
    bool isValidEmail(const QString& email);
    bool isValidPassword(const QString& password);
    bool isValidUsername(const QString& username);
    void updatePasswordStatistics();
    QString hashPassword(const QString& password);
    
    // Password CRUD Operations
    void addPassword();
    void loadPasswords(); 
    void deletePassword(int id);
    void editPassword(int id);
	void searchPasswords(const QString& keyword);
    void addPasswordRow(int row, const QSqlQuery& query);
	int findRowByPasswordId(int passwordId);
    void changeMasterPassword();

	// settings & auto lock
    void handleAutoLock(bool enabled);
    void autoLockVault();

	// clean up input fields
    void clearPasswordInputs();
    void clearLoginRegisterInputs();

private:
    Ui::HashVaultClass ui;
	//QSqlDatabase db;                // create global database object to be used across the application
    QTimer* autoLockTimer;

    int currentUserId = -1;         // store the logged-in user's id
    int editingPasswordId = -1;     // store the id of the password entry being edited

    void setupAuthConnections();
    void setupPasswordConnections();
    void setupSettingsConnections();
    void setupHelperConnections();
};

