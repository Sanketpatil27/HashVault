#include "../HashVault.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QInputDialog>

void HashVault::setupAuthConnections()
{
    connect(ui.registerBtn, &QPushButton::clicked, this, &HashVault::registerUser);                 // adding to database
    connect(ui.loginButton, &QPushButton::clicked, this, &HashVault::loginUser);                    // validating user credentials and then redirecting to dashboard
    connect(ui.navLogout, &QPushButton::clicked, this, &HashVault::logoutUser);
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

    // username validation
    if (!isValidUsername(username))
    {
        return;
    }

    // email validation
    if (!isValidEmail(email))
    {
        return;
    }

    // password validation
    if (!isValidPassword(password))
    {
        QMessageBox::warning(
            this,
            "Weak Password",
            "Password must contain:\n"
            "- At least 8 characters\n"
            "- One uppercase letter\n"
            "- One lowercase letter\n"
            "- One number\n"
            "- One special character"
        );
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

    query.addBindValue(fullname);
    query.addBindValue(username);
    query.addBindValue(email);
    query.addBindValue(hashPassword(password));

    if (query.exec()) {
        //QMessageBox::information(this, "Success", "User registered successfully!");
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
    query.addBindValue(hashPassword(password));

    if (query.exec() && query.next()) {
        currentUserId = query.value("id").toInt();          // store the logged-in user's ID for later use like fetching user-specific passwords

        //QMessageBox::information(this, "Success", "Login successful!");

        loadPasswords();
        loadCategories();
        
        updatePasswordStatistics();

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

    //QMessageBox::information(this, "Logout", "Logged out successfully!");

    ui.stackedWidget->setCurrentWidget(ui.loginPage);
}

