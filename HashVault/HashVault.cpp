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

	db = QSqlDatabase::addDatabase("QPSQL");

	db.setHostName("localhost");
	db.setDatabaseName("hashvault");
	db.setUserName("postgres");
	db.setPassword("root");
	db.setPort(5432);

	//check database connection & open the database for executing queries
    if (db.open())
        QMessageBox::information(this, "Database", "DB Connected Successfully!");
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

    
    connect(ui.navSettings, &QPushButton::clicked, this, &HashVault::openSettings);
    connect(ui.addPasswordBtn, &QPushButton::clicked, this, &HashVault::openAddPasswordPage);
    connect(ui.backToDashboardBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromAddPage);
    connect(ui.settingsBackBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromSettings);

    // Redirect to Register Page
    connect(ui.registerRedirectBtn, &QPushButton::clicked, this, &HashVault::openRegisterPage);

    // Redirect back to Login Page
    connect(ui.loginRedirectBtn, &QPushButton::clicked, this, &HashVault::openLoginPage);

	connect(ui.registerBtn, &QPushButton::clicked, this, &HashVault::registerUser);     // adding to database
	connect(ui.loginButton, &QPushButton::clicked, this, &HashVault::loginUser);        // validating user credentials and then redirecting to dashboard

}

HashVault::~HashVault()
{}


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

void HashVault::registerUser() {
	QString fullname = ui.registerFullNameInput->text();
	QString username = ui.registerUsernameInput->text();
    QString email = ui.registerEmailInput->text();
    QString password = ui.registerPasswordInput->text();
    QString confirmPassword = ui.registerConfirmPasswordInput->text();

    if (fullname.isEmpty() || username.isEmpty() || email.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    if (password != confirmPassword)
    {
        QMessageBox::warning(this, "Error", "Passwords do not match");
        return;
    }

	// if validation is successful then inserting user data into database
    QSqlQuery query;
	query.prepare("INSERT INTO users (fullname, username, email, password) VALUES (?, ?, ?, ?)");

    // giving error while executing query
	//query.bindValue(":fullname", fullname);
    //query.bindValue(":username", username);
	//query.bindValue(":email", email);
	//query.bindValue(":password", password); // will hash the password before storing in later submissions

    query.addBindValue(fullname);
    query.addBindValue(username);
    query.addBindValue(email);
    query.addBindValue(password); // will hash the password before storing in later submissions

    if (query.exec()) {
		QMessageBox::information(this, "Success", "User registered successfully!");
		ui.stackedWidget->setCurrentWidget(ui.loginPage); // Redirect to login page after successful registration
	}
    else {
        QMessageBox::critical(this, "Error", "Failed to register user: " + query.lastError().text());
    }
}

void HashVault::loginUser() {
	QString username = ui.loginUsernameInput->text();
	QString password = ui.loginPasswordInput->text();
	
    if (username.isEmpty() || password.isEmpty())
	{
		QMessageBox::warning(this, "Error", "Please enter both username and password");
		return;
	}

	QSqlQuery query;
	query.prepare("SELECT * FROM users WHERE username = ? AND password = ?"); // will hash the password before comparing in later submissions
	query.addBindValue(username);
	query.addBindValue(password);

	if (query.exec() && query.next()) {
		QMessageBox::information(this, "Success", "Login successful!");
		ui.stackedWidget->setCurrentWidget(ui.dashboardPage);               // Redirect to dashboard page after successful login
	}
	else {
		QMessageBox::critical(this, "Error", "Invalid username or password");
	}
}