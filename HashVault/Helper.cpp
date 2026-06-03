#include "HashVault.h"
#include <qmessagebox.h>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>


void HashVault::setupHelperConnections()
{
    // navigation connectors
    connect(ui.navSettings, &QPushButton::clicked, this, &HashVault::openSettings);
    connect(ui.addPasswordBtn, &QPushButton::clicked, this, &HashVault::openAddPasswordPage);
    connect(ui.cancelAddBtn, &QPushButton::clicked, this, &HashVault::cancelPasswordEditing);
    connect(ui.settingsBackBtn, &QPushButton::clicked, this, &HashVault::backToDashboardFromSettings);
    connect(ui.registerRedirectBtn, &QPushButton::clicked, this, &HashVault::openRegisterPage);         // Redirect to Register Page from login page
    connect(ui.loginRedirectBtn, &QPushButton::clicked, this, &HashVault::openLoginPage);               // Redirect back to Login Page from register page

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

//------------------- Navigation implementation ---------------------------
void HashVault::openSettings() { ui.stackedWidget->setCurrentWidget(ui.settingsPage); }
void HashVault::openAddPasswordPage() { ui.stackedWidget->setCurrentWidget(ui.addPasswordPage); }
void HashVault::backToDashboardFromSettings() { ui.stackedWidget->setCurrentWidget(ui.dashboardPage); }
void HashVault::openRegisterPage() { ui.stackedWidget->setCurrentWidget(ui.registerPage); }
void HashVault::openLoginPage() { ui.stackedWidget->setCurrentWidget(ui.loginPage); }
void HashVault::cancelPasswordEditing() {
    clearPasswordInputs();
    ui.stackedWidget->setCurrentWidget(ui.dashboardPage);
}


// ---------- making passwords visible -----------------------
void HashVault::togglePasswordVisibility(QLineEdit* field, bool checked) {
    if (checked)
        field->setEchoMode(QLineEdit::Normal);
    else
        field->setEchoMode(QLineEdit::Password);
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

// ------------ validation  ------------------------
bool HashVault::isValidEmail(const QString& email)
{
    // format validation
    QRegularExpression regex(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    if (!regex.match(email).hasMatch())
    {
        QMessageBox::warning(this, "Invalid Email", "Please enter a valid email address.");
        return false;
    }

    // Check if email already exists
    QSqlQuery query;

    query.prepare("SELECT 1 FROM users WHERE email = ?");

    query.addBindValue(email);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return false;
    }

    if (query.next())
    {
        QMessageBox::warning(this, "Email Exists", "This email is already registered.");
        return false;
    }

    return true;
}

bool HashVault::isValidPassword(const QString& password)
{
    if (password.length() < 8)
        return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (QChar ch : password)   
    {
        if (ch.isUpper())
            hasUpper = true;

        else if (ch.isLower())
            hasLower = true;

        else if (ch.isDigit())
            hasDigit = true;

        else
            hasSpecial = true;
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

bool HashVault::isValidUsername(const QString& username)
{
    if (username.length() < 4)
    {
        QMessageBox::warning(this, "Invalid Username", "Username must be at least 4 characters long.");
        return false;
    }

    QSqlQuery query;

    query.prepare("SELECT 1 FROM users WHERE username = ?");

    query.addBindValue(username);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return false;
    }

    if (query.next())
    {
        QMessageBox::warning(this, "Username Exists", "Username is already taken.");
        return false;
    }

    return true;
}

QString HashVault::hashPassword(const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

    return hash.toHex();
}


//----- Password Status Helper -----
void HashVault::updatePasswordStatistics()
{
    int totalPasswords = 0;
    int strongPasswords = 0;
    int weakPasswords = 0;

    for (int row = 0; row < ui.passwordTable->rowCount(); row++)
    {
        QString password =
            ui.passwordTable->item(row, 3)->text();

        totalPasswords++;

        if (isValidPassword(password))
            strongPasswords++;
        else
            weakPasswords++;
    }

    ui.totalPasswordsValue->setText(QString::number(totalPasswords));

    ui.strongPasswordsValue->setText(QString::number(strongPasswords));

    ui.weakPasswordsValue->setText(QString::number(weakPasswords));
}