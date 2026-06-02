#include "../HashVault.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QInputDialog>

void HashVault::setupPasswordConnections() {
    // Crud operations
    connect(ui.savePasswordBtn, &QPushButton::clicked, this, &HashVault::addPassword);              // adding password to database and then refreshing the dashboard table to show the newly added password
    connect(ui.searchInput, &QLineEdit::textChanged, this, &HashVault::searchPasswords);
    connect(ui.changeMasterPwdBtn, &QPushButton::clicked, this, &HashVault::changeMasterPassword);   // the the main master password of user

    // hidden column for id of the password
    ui.passwordTable->setColumnHidden(0, true);
    // setting default height for row in password table
    ui.passwordTable->verticalHeader()->setDefaultSectionSize(54);
    // adding extra width to notes column(index 4, 0th col is hidden)
    ui.passwordTable->setColumnWidth(4, 280);
    // width for actions column
    ui.passwordTable->setColumnWidth(5, 140);
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

void HashVault::searchPasswords(const QString& searchText) {
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

    if (query.exec())
    {
        int row = 0;
        while (query.next())
        {
            addPasswordRow(row, query);
            row++;
        }
    }
}

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
