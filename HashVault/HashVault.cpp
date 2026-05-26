#include "HashVault.h"
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

    // Adding AUTO LOCK TIMER
    autoLockTimer = new QTimer(this);
    connect(ui.autoLockCheckBox,&QCheckBox::toggled, this, &HashVault::handleAutoLock);
    connect(autoLockTimer, &QTimer::timeout, this, &HashVault::autoLockVault);

    // hidden column for id of the password
    ui.passwordTable->setColumnHidden(0, true);
	// setting default height for row in password table
    ui.passwordTable->verticalHeader()->setDefaultSectionSize(54);
    // adding extra width to notes column(index 4, 0th col is hidden)
    ui.passwordTable->setColumnWidth(4, 280);
	// width for actions column
    ui.passwordTable->setColumnWidth(5, 140);

    // connecting to the database
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

	// navigation connectors
    connect(ui.navSettings, &QPushButton::clicked, this, &HashVault::openSettings);
    connect(ui.addPasswordBtn, &QPushButton::clicked, this, &HashVault::openAddPasswordPage);
    connect(ui.cancelAddBtn, &QPushButton::clicked,this, &HashVault::cancelPasswordEditing);
    connect(ui.settingsBackBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromSettings);
    connect(ui.registerRedirectBtn, &QPushButton::clicked, this, &HashVault::openRegisterPage);         // Redirect to Register Page from login page
    connect(ui.loginRedirectBtn, &QPushButton::clicked, this, &HashVault::openLoginPage);               // Redirect back to Login Page from register page

    // auth connectors 
    connect(ui.registerBtn, &QPushButton::clicked, this, &HashVault::registerUser);                 // adding to database
    connect(ui.loginButton, &QPushButton::clicked, this, &HashVault::loginUser);                    // validating user credentials and then redirecting to dashboard
    connect(ui.navLogout, &QPushButton::clicked, this, &HashVault::logoutUser);

    // Crud operations
    connect(ui.savePasswordBtn, &QPushButton::clicked, this, &HashVault::addPassword);              // adding password to database and then refreshing the dashboard table to show the newly added password
    connect(ui.searchInput, &QLineEdit::textChanged, this, &HashVault::searchPasswords);
    connect(ui.changeMasterPwdBtn, &QPushButton::clicked, this, &HashVault::changeMasterPassword);   // the the main master password of user


    // change password toggles
	connect(ui.loginShowPasswordCheckBox, &QCheckBox::toggled, this, 
        [=](bool checked) 
        {
		    togglePasswordVisibility(ui.loginPasswordInput, checked);
	    }
    );

    connect(ui.registerShowPasswordCheckBox, &QCheckBox::toggled, this,
        [=](bool checked)
        {
            togglePasswordVisibility(ui.registerPasswordInput, checked);
            togglePasswordVisibility(ui.registerConfirmPasswordInput, checked);
        }
    );

    connect(ui.showStoredPasswordCheckBox, &QCheckBox::toggled, this,
        [=](bool checked)
        {
            togglePasswordVisibility(ui.passwordFormInput, checked);
        }
    );    
}

HashVault::~HashVault()
{}

// --------------------------------- Navigation implementation ---------------------------

void HashVault::openSettings()                  { ui.stackedWidget->setCurrentWidget(ui.settingsPage); }
void HashVault::openAddPasswordPage()           { ui.stackedWidget->setCurrentWidget(ui.addPasswordPage); }
void HashVault::backToDashboardFromSettings()   { ui.stackedWidget->setCurrentWidget(ui.dashboardPage); }
void HashVault::openRegisterPage()              { ui.stackedWidget->setCurrentWidget(ui.registerPage); }
void HashVault::openLoginPage()                 { ui.stackedWidget->setCurrentWidget(ui.loginPage); }
void HashVault::cancelPasswordEditing()
{
    clearPasswordInputs();

    ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}



// ------------------------------- Auth Implementation ------------------------------------

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

    // this is giving error while executing query (search later)
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

        clearLoginRegisterInputs();     // clear the input fields from login/register

		ui.stackedWidget->setCurrentWidget(ui.dashboardPage);               // Redirect to dashboard page after successful login
	}
	else {
		QMessageBox::critical(this, "Error", "Invalid username or password");
	}
}

void HashVault::logoutUser() {
    currentUserId = -1;
    autoLockTimer->stop();

    QMessageBox::information(this, "Logout", "Logged out successfully!");

    ui.stackedWidget->setCurrentWidget(ui.loginPage);
}



// ---------------------------- Password CRUD operations ----------------------------------

void HashVault::addPassword() {
    QString website = ui.websiteInput->text();
    QString username = ui.usernameFormInput->text();
    QString password = ui.passwordFormInput->text();
	QString notes = ui.notesInput->toPlainText();

    if (website.isEmpty() || username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    QSqlQuery query;

    // -1 means adding new entry
    if (editingPasswordId == -1) {
        query.prepare("insert into passwords (website, username, password, notes, user_id) values (?, ?, ?, ?, ?)");

        query.addBindValue(website);
        query.addBindValue(username);
        query.addBindValue(password);
        query.addBindValue(notes);
        query.addBindValue(currentUserId); // associate the password entry with the logged-in user
    }

    // query for updating password 
    else {
        query.prepare(
            "UPDATE passwords "
            "SET website = ?, "
            "username = ?, "
            "password = ?, "
            "notes = ? "
            "WHERE id = ?"
        );

        query.addBindValue(website);
        query.addBindValue(username);
        query.addBindValue(password);
        query.addBindValue(notes);
        query.addBindValue(editingPasswordId);
    }


    if (query.exec()) {
        QString successMessage = (editingPasswordId == -1) 
            ? "Password added successfully!"
            : "Password updated successfully!";
		
        QMessageBox::information(this, "Success", successMessage);

        clearPasswordInputs();

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

	query.prepare("SELECT * FROM passwords WHERE user_id = ? ORDER BY id");
	query.addBindValue(currentUserId);

    if (query.exec()) {
		int row = 0;

        while (query.next()) {
            addPasswordRow(row, query);
			row++;
        }
    }
}

void HashVault::deletePassword(int id) {
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(
        this,
        "Delete Password",
        "Are you sure you want to delete this password?",
        QMessageBox::Yes | QMessageBox::No
    );

    // if user clicks NO then do nothing
    if (reply == QMessageBox::No)
        return;

    // if user selected yes then prepare query for deleting the row
    QSqlQuery query;

	query.prepare("DELETE FROM passwords WHERE id = ?");
	query.addBindValue(id);

    if (query.exec()) {
		loadPasswords(); // Refreshing the table after deletion
    }
    else {
		QMessageBox::warning(this, "Error", query.lastError().text());
    }
}

void HashVault::editPassword(int id) {
	editingPasswordId = id; // storing the id of the password entry being edited to identify which entry to update in database when save button is clicked

	QSqlQuery query;
	query.prepare("SELECT * FROM passwords WHERE id = ?");
	query.addBindValue(id);

	// set the current values of the password entry to the form inputs in add password page for editing
    if (query.exec() && query.next())
    {
        ui.websiteInput->setText(query.value(1).toString());
        ui.usernameFormInput->setText(query.value(2).toString());
        ui.passwordFormInput->setText(query.value(3).toString());
        ui.notesInput->setPlainText(query.value(6).toString());      // notes column in database is at 6th index

        ui.stackedWidget->setCurrentWidget(ui.addPasswordPage);
    }
}

void HashVault::searchPasswords(const QString &searchText) {
    ui.passwordTable->setRowCount(0);

    QSqlQuery query;

    query.prepare(
        "SELECT * FROM passwords "
        "WHERE user_id = ? "
        "AND LOWER(website) LIKE LOWER(?) "
        "ORDER BY id"
    );

    query.addBindValue(currentUserId);
    query.addBindValue("%" + searchText + "%");

    if(query.exec())
    {
        int row = 0;
        while (query.next())
        {
            addPasswordRow(row, query);
            row++;
        }
    }
}

// helper function for adding rows into table
void HashVault::addPasswordRow(int row, const QSqlQuery& query)
{
    ui.passwordTable->insertRow(row);

    // column 0 is hidden and used to store the id of the password entry
    ui.passwordTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));

    // visible columns in the table for website, username, password and notes
    ui.passwordTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString())); // value at 1(column no.) is website in database
    ui.passwordTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString())); // username
    ui.passwordTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString())); // password
    ui.passwordTable->setItem(row, 4, new QTableWidgetItem(query.value(6).toString())); // notes

    // adding edit and delete buttons to each row in the table
    QWidget* actionWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(actionWidget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignCenter);

    QPushButton* editBtn = new QPushButton("Edit");
    QPushButton* deleteBtn = new QPushButton("Del");

    editBtn->setFixedSize(55, 30);
    deleteBtn->setFixedSize(55, 30);

    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);

    // styling the buttons
    editBtn->setStyleSheet(
        "QPushButton {"
        "background-color:#3A7BD5;"
        "color:white;"
        "border:none;"
        "border-radius:8px;"
        "font-size:11px;"
        "font-weight:bold;"
        "padding:4px 8px;"
        "}"
    );

    deleteBtn->setStyleSheet(
        "QPushButton {"
        "background-color:#E05C5C;"
        "color:white;"
        "border:none;"
        "border-radius:8px;"
        "font-size:11px;"
        "font-weight:bold;"
        "padding:4px 8px;"
        "}"
    );

    // adding button to layout
    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);
    actionWidget->setLayout(layout);

    // adding to actions column
    ui.passwordTable->setCellWidget(row, 5, actionWidget); // 4 is the column number for actions, first column is hidden


    // connecting buttons for edit and delete functionality by passing passwordId of current password row
    int passwordId = query.value(0).toInt(); // get the id of the password entry from database to identify which entry to edit/delete when buttons are clicked

    connect(deleteBtn, &QPushButton::clicked, this, [=]() {
        deletePassword(passwordId);
    });

    connect(editBtn, &QPushButton::clicked, this, [=]() {
        editPassword(passwordId);
    });
}

// making passwords visible
void HashVault::togglePasswordVisibility(QLineEdit* field, bool checked) {
	if (checked)
		field->setEchoMode(QLineEdit::Normal);
	else 
		field->setEchoMode(QLineEdit::Password);
}

void HashVault::changeMasterPassword() {
    bool ok;

    QString currentPassword = QInputDialog::getText(this, "Current Password", "Enter Current Password", QLineEdit::Normal, "", &ok);

    if (!ok || currentPassword.isEmpty())
        return;

    // verify the current password
    QSqlQuery verifyQuery;

	verifyQuery.prepare("SELECT password FROM users WHERE id = ? AND password = ?");
    verifyQuery.addBindValue(currentUserId);
	verifyQuery.addBindValue(currentPassword); // will hash the password before comparing in later submissions

    // if password is not matching then give warning
    if (!(verifyQuery.exec() && verifyQuery.next()))
    {
        QMessageBox::warning(this, "Error", "Current password is incorrect");
        return;
    }

    // ask for new password if matched
    QString newPassword = QInputDialog::getText(this, "New Password", "Enter new master password:", QLineEdit::Normal, "", &ok);

    if (!ok || newPassword.isEmpty())
        return;
    
    // now update the password
    QSqlQuery updateQuery;

    updateQuery.prepare("UPDATE users SET password = ? WHERE id = ?");

    updateQuery.addBindValue(newPassword);
    updateQuery.addBindValue(currentUserId);

    if (updateQuery.exec())
        QMessageBox::information(this, "Success", "Master password updated successfully!");
    else
        QMessageBox::warning(this, "Error", updateQuery.lastError().text());
}

void HashVault::handleAutoLock(bool enabled)
{
    if (enabled)
        autoLockTimer->start(10000);
    else
        autoLockTimer->stop();
}

void HashVault::autoLockVault()
{
    QMessageBox::information(this, "Vault Locked", "Session expired due to inactivity");
    logoutUser();
}



// ------------------- clear input fields -----------------------

void HashVault::clearPasswordInputs() {
    ui.websiteInput->clear();
    ui.usernameFormInput->clear();
    ui.passwordFormInput->clear();
    ui.notesInput->clear();

    editingPasswordId = -1;         // reseat the editing mode
}

void HashVault::clearLoginRegisterInputs()
{
    ui.loginUsernameInput->clear();
    ui.loginPasswordInput->clear();
    ui.registerFullNameInput->clear();
    ui.registerUsernameInput->clear();
    ui.registerEmailInput->clear();
    ui.registerPasswordInput->clear();
    ui.registerConfirmPasswordInput->clear();
}