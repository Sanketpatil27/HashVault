#include "../HashVault.h"
#include <openssl/rand.h>
#include <qcryptographichash.h>
#include "../security/CryptoManager.h"
#include <qsettings.h>

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
    query.prepare(
        "INSERT INTO users "
        "(fullname, username, email, password, salt, encrypted_dek) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(fullname);
    query.addBindValue(username);
    query.addBindValue(email);

    QString salt = generateSalt();
    QString hashedPassword = hashPassword(password, salt);

    query.addBindValue(hashedPassword);
    query.addBindValue(salt);
    
    QByteArray dek(32, 0);

    RAND_bytes(
        reinterpret_cast<unsigned char*>(dek.data()),
        32
    );

    QByteArray kek =
        QCryptographicHash::hash(
            (hashedPassword + salt).toUtf8(),
            QCryptographicHash::Sha256
        );

    CryptoManager::setCurrentKey(kek);
    
    QString encryptedDek = CryptoManager::encrypt(dek.toBase64());

    query.addBindValue(encryptedDek);

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

    query.prepare(
        "SELECT * FROM users "
        "WHERE username = ?"
    );

    query.addBindValue(username);

    if (query.exec() && query.next())
    {
        QString storedHash = query.value("password").toString();

        QString salt = query.value("salt").toString();

        QString enteredHash = hashPassword(password, salt);

        if (storedHash == enteredHash)
        {
            QString encryptedDek = query.value("encrypted_dek").toString();

            QByteArray kek =
                QCryptographicHash::hash(
                    (storedHash + salt).toUtf8(),
                    QCryptographicHash::Sha256
                );

            CryptoManager::setCurrentKey(kek);

            currentUserDek =
                QByteArray::fromBase64(
                    CryptoManager::decrypt(
                        encryptedDek
                    ).toUtf8()
                );

            CryptoManager::setCurrentKey(currentUserDek);

            currentUserId = query.value("id").toInt();

            QSettings settings("HashVault", "PasswordManager");

            settings.setValue("loggedIn", true);

            settings.setValue("userId", currentUserId);

            settings.setValue("userDek", currentUserDek.toBase64());

            loadPasswords();
            loadCategories();

            updatePasswordStatistics();

            clearLoginRegisterInputs();

            ui.stackedWidget->setCurrentWidget(ui.dashboardPage);

            return;
        }
    }

    QMessageBox::critical(
        this,
        "Error",
        "Invalid username or password"
    );
}

void HashVault::logoutUser() {
    QSettings settings("HashVault", "PasswordManager");
    settings.remove("loggedIn");
    settings.remove("userId");
    settings.remove("userDek");
    currentUserDek.clear();

    currentUserId = -1;
    autoLockTimer->stop();

    currentUserDek.clear();

    CryptoManager::setCurrentKey(QByteArray());


    ui.stackedWidget->setCurrentWidget(ui.loginPage);
}


QString HashVault::generateSalt()
{
    QByteArray salt(16, 0);

    RAND_bytes(
        reinterpret_cast<unsigned char*>(
            salt.data()
            ),
        16
    );

    return salt.toHex();
}

QString HashVault::hashPassword(const QString& password, const QString& salt)
{
    QByteArray hash = 
        QCryptographicHash::hash(
            (password + salt).toUtf8(),
            QCryptographicHash::Sha256
        );

    return hash.toHex();
}