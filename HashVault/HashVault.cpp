#include "HashVault.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <qmessagebox.h>

HashVault::HashVault(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

	db.setHostName("localhost");
	db.setDatabaseName("hashvault");
	db.setUserName("postgres");
	db.setPassword("root");
	db.setPort(5432);


    if (db.open())
        QMessageBox::information(this, "Database", "Connected Successfully!");
    else
        QMessageBox::warning(this,"Database Error",db.lastError().text());



    // add dummy entries in dashboard table 
    ui.passwordTable->horizontalHeader()->setDefaultSectionSize(180);
    ui.passwordTable->setRowCount(3);

    ui.passwordTable->setItem(0, 0, new QTableWidgetItem("Google"));
    ui.passwordTable->setItem(0, 1, new QTableWidgetItem("sanket@gmail.com"));
    ui.passwordTable->setItem(0, 2, new QTableWidgetItem("********"));

    ui.passwordTable->setItem(1, 0, new QTableWidgetItem("GitHub"));
    ui.passwordTable->setItem(1, 1, new QTableWidgetItem("sanketdev"));
    ui.passwordTable->setItem(1, 2, new QTableWidgetItem("********"));

    ui.passwordTable->setItem(2, 0, new QTableWidgetItem("LinkedIn"));
    ui.passwordTable->setItem(2, 1, new QTableWidgetItem("sanket.patil"));
    ui.passwordTable->setItem(2, 2, new QTableWidgetItem("********"));

    
    connect(ui.loginButton, &QPushButton::clicked, this, &HashVault::handleLogin);
    connect(ui.navSettings, &QPushButton::clicked, this, &HashVault::openSettings);
    connect(ui.addPasswordBtn, &QPushButton::clicked, this, &HashVault::openAddPasswordPage);
    connect(ui.backToDashboardBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromAddPage);
    connect(ui.settingsBackBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromSettings);

    // Redirect to Register Page
    connect(ui.registerRedirectBtn, &QPushButton::clicked, this, &HashVault::openRegisterPage);

    // Redirect back to Login Page
    connect(ui.loginRedirectBtn, &QPushButton::clicked, this, &HashVault::openLoginPage);

}

HashVault::~HashVault()
{}



void HashVault::handleLogin() {
    //QMessageBox::information(this, "login", "Login Button Clicked");
    ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}

void HashVault::openSettings()
{
    ui.stackedWidget->setCurrentWidget(ui.settingsPage);
}

void HashVault::openAddPasswordPage()
{
    ui.stackedWidget->setCurrentWidget(ui.addPasswordPage);
}

void HashVault::backToDashboardFromAddPage()
{
    ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}

void HashVault::backToDashboardFromSettings()
{
    ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}

void HashVault::openRegisterPage()
{
    ui.stackedWidget->setCurrentWidget(ui.registerPage);
}

void HashVault::openLoginPage()
{
    ui.stackedWidget->setCurrentWidget(ui.loginPage);
}