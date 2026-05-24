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

    
    connect(ui.navSettings, &QPushButton::clicked, this, &HashVault::openSettings);
    connect(ui.addPasswordBtn, &QPushButton::clicked, this, &HashVault::openAddPasswordPage);
    connect(ui.backToDashboardBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromAddPage);
    connect(ui.settingsBackBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromSettings);

    // Redirect to Register Page
    connect(ui.registerRedirectBtn, &QPushButton::clicked, this, &HashVault::openRegisterPage);

    // Redirect back to Login Page
    connect(ui.loginRedirectBtn, &QPushButton::clicked, this, &HashVault::openLoginPage);

	connect(ui.registerBtn, &QPushButton::clicked, this, &HashVault::registerUser);                 // adding to database
	connect(ui.loginButton, &QPushButton::clicked, this, &HashVault::loginUser);                    // validating user credentials and then redirecting to dashboard

	connect(ui.savePasswordBtn, &QPushButton::clicked, this, &HashVault::addPassword);              // adding password to database and then refreshing the dashboard table to show the newly added password

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
		currentUserId = query.value("id").toInt();          // store the logged-in user's ID for later use like fetching user-specific passwords

		QMessageBox::information(this, "Success", "Login successful!");
        
        loadPasswords();

		ui.stackedWidget->setCurrentWidget(ui.dashboardPage);               // Redirect to dashboard page after successful login
	}
	else {
		QMessageBox::critical(this, "Error", "Invalid username or password");
	}
}

void HashVault::addPassword() {
    QString website = ui.websiteInput->text();
    //QString url = ui.urlInput->text();
    QString username = ui.usernameFormInput->text();
    QString password = ui.passwordFormInput->text();
	QString notes = ui.notesInput->toPlainText();

    if (website.isEmpty() || username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    QSqlQuery query;
	query.prepare("insert into passwords (website, username, password, notes, user_id) values (?, ?, ?, ?, ?)");

	query.addBindValue(website);
	query.addBindValue(username);
	query.addBindValue(password); 
    query.addBindValue(notes);
	query.addBindValue(currentUserId); // associate the password entry with the logged-in user

    if (query.exec()) {
		QMessageBox::information(this, "Success", "Password added successfully!");

        ui.websiteInput->clear();
		ui.usernameFormInput->clear();
		ui.passwordFormInput->clear();
		ui.notesInput->clear();

        loadPasswords();
		ui.stackedWidget->setCurrentWidget(ui.dashboardPage); // Redirect back to dashboard after adding password
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to add password: " + query.lastError().text());
    }
}

void HashVault::loadPasswords() {
	ui.passwordTable->setRowCount(0); // Clear existing rows

    QSqlQuery query;

	query.prepare("SELECT * FROM passwords WHERE user_id = ?");
	query.addBindValue(currentUserId);

    if (query.exec()) {
		int row = 0;

        while (query.next()) {
			ui.passwordTable->insertRow(row);

			ui.passwordTable->setItem(row, 0, new QTableWidgetItem(query.value(1).toString())); // value at 1(column no.) is website in database
			ui.passwordTable->setItem(row, 1, new QTableWidgetItem(query.value(2).toString())); // username
			ui.passwordTable->setItem(row, 2, new QTableWidgetItem(query.value(3).toString())); // password
			ui.passwordTable->setItem(row, 3, new QTableWidgetItem(query.value(6).toString())); // notes

			row++;
        }
    }
}