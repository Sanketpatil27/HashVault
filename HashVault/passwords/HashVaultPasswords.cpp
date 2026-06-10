#include "../HashVault.h"
#include "../security/CryptoManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QInputDialog>

void HashVault::setupPasswordConnections() {
    // Crud operations
    connect(ui.savePasswordBtn, &QPushButton::clicked, this, &HashVault::addPassword);              // adding password to database and then refreshing the dashboard table to show the newly added password
    connect(ui.searchInput, &QLineEdit::textChanged, this, &HashVault::searchPasswords);
    connect(ui.changeMasterPwdBtn, &QPushButton::clicked, this, &HashVault::changeMasterPassword);   // the the main master password of user

	// category management
    connect(ui.categoryFilterCombo, &QComboBox::currentTextChanged, this,
        &HashVault::filterPasswordsByCategory
    );

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
        //loadPasswords(); // Refreshing the table after deletion
        int row = findRowByPasswordId(id);

        if (row != -1)
            ui.passwordTable->removeRow(row);

        updatePasswordStatistics();
        loadCategories();
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
        ui.passwordFormInput->setText(CryptoManager::decrypt(query.value(3).toString()));
        ui.notesInput->setPlainText(query.value(6).toString());      // notes column in database is at 6th index
        ui.categoryInput->setCurrentText(query.value("category").toString());

        ui.stackedWidget->setCurrentWidget(ui.addPasswordPage);
    }
}

void HashVault::searchPasswords(const QString& searchText) {

    for (int row = 0; row < ui.passwordTable->rowCount(); row++)
    {
        QTableWidgetItem* websiteItem = ui.passwordTable->item(row, 1);

        if (!websiteItem)
            continue;

        QString website = websiteItem->text();

        bool match = website.contains(searchText, Qt::CaseInsensitive);

        ui.passwordTable->setRowHidden(row, !match);
    }
}

void HashVault::addPassword() {
    QString website = ui.websiteInput->text();
    QString username = ui.usernameFormInput->text();
    QString password = ui.passwordFormInput->text();
    QString notes = ui.notesInput->toPlainText();
    QString category = ui.categoryInput->lineEdit()->text().trimmed();

    if (website.isEmpty() || username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    QSqlQuery query;

    // -1 means adding new entry
    if (editingPasswordId == -1) {
        query.prepare("insert into passwords (website, username, password, notes, user_id, category) values (?, ?, ?, ?, ?, ?)");

        query.addBindValue(website);
        query.addBindValue(username);
        query.addBindValue(CryptoManager::encrypt(password));
        query.addBindValue(notes);
        query.addBindValue(currentUserId); // associate the password entry with the logged-in user
        query.addBindValue(category);

        if (query.exec())
        {
            QSqlQuery fetchQuery;

            fetchQuery.prepare("SELECT * FROM passwords WHERE user_id = ? ORDER BY id DESC LIMIT 1");
            fetchQuery.addBindValue(currentUserId);

            if (fetchQuery.exec() && fetchQuery.next())
            {
                // Add new row in table
                int newRow = ui.passwordTable->rowCount();

                addPasswordRow(newRow, fetchQuery);
                loadCategories();
                QMessageBox::information(this, "Success", "Password added successfully!");
            }
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to add password: " + query.lastError().text());
        }
    }

    // query for updating password 
    else {
        query.prepare(
            "UPDATE passwords "
            "SET website = ?, "
            "username = ?, "
            "password = ?, "
            "notes = ?, "
			"category = ? "
            "WHERE id = ?"
        );

        query.addBindValue(website);
        query.addBindValue(username);
        query.addBindValue(CryptoManager::encrypt(password));
        query.addBindValue(notes);
        query.addBindValue(category);
        query.addBindValue(editingPasswordId);

        if (query.exec()) {
            // UPDATE EXISTING ROW
            int row = findRowByPasswordId(editingPasswordId);
            
            if (row != -1)
            {
                ui.passwordTable->item(row, 1)->setText(website);
                ui.passwordTable->item(row, 2)->setText(username);
                ui.passwordTable->item(row, 3)->setText(password);
                ui.passwordTable->item(row, 4)->setText(notes);
                ui.passwordTable->item(row, 5)->setText(ui.categoryInput->lineEdit()->text().trimmed());
            }

            QMessageBox::information(this, "Success", "Password updated successfully!");
            loadCategories();
        }

        else {
            QMessageBox::critical(this, "Error", "Failed to add password: " + query.lastError().text());
        }
    }
    
    updatePasswordStatistics();
    clearPasswordInputs();
    ui.stackedWidget->setCurrentWidget(ui.dashboardPage); // Redirect back to dashboard after adding password
}


void HashVault::loadPasswords() {
    ui.passwordTable->setRowCount(0); // Clear existing rows

    QSqlQuery query;

    //query.prepare("SELECT * FROM passwords WHERE user_id = ? ORDER BY id");
    query.prepare("SELECT * FROM passwords WHERE user_id = ?");
    query.addBindValue(currentUserId);

    if (query.exec()) {
        int row = 0;

        while (query.next()) {
            addPasswordRow(row, query);
            row++;
        }
    }
}

// find row by id
int HashVault::findRowByPasswordId(int passwordId)
{
    for (int row = 0; row < ui.passwordTable->rowCount(); row++)
    {
        int id = ui.passwordTable->item(row, 0)->text().toInt();

        if (id == passwordId)
            return row;
    }

    return -1;
}


// helper function for adding rows into table
void HashVault::addPasswordRow(int row, const QSqlQuery& query)
{
    QString decryptedPassword = CryptoManager::decrypt(query.value(3).toString());

    ui.passwordTable->insertRow(row);

    // column 0 is hidden and used to store the id of the password entry
    ui.passwordTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));

    // visible columns in the table for website, username, password and notes
    ui.passwordTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString())); // value at 1(column no.) is website in database
    ui.passwordTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString())); // username
    ui.passwordTable->setItem(row, 3, new QTableWidgetItem(decryptedPassword)); // password
    ui.passwordTable->setItem(row, 4, new QTableWidgetItem(query.value(6).toString())); // notes

    QString category = query.value("category").toString();
    ui.passwordTable->setItem(row, 5, new QTableWidgetItem(category));

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
    ui.passwordTable->setCellWidget(row, 6, actionWidget); // 4 is the column number for actions, first column is hidden


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
    verifyQuery.addBindValue(hashPassword(currentPassword));

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

    updateQuery.addBindValue(hashPassword(newPassword));
    updateQuery.addBindValue(currentUserId);

    if (updateQuery.exec())
        QMessageBox::information(this, "Success", "Master password updated successfully!");
    else
        QMessageBox::warning(this, "Error", updateQuery.lastError().text());
}

void HashVault::loadCategories()
{
    ui.categoryFilterCombo->clear();
    ui.categoryInput->clear();

    ui.categoryFilterCombo->addItem("All Categories");

    QSqlQuery query;

    query.prepare(
        "SELECT DISTINCT category "
        "FROM passwords "
        "WHERE user_id = ? "
        "AND category IS NOT NULL "
        "AND category <> '' "
        "ORDER BY category"
    );

    query.addBindValue(currentUserId);

    if (query.exec())
    {
        while (query.next())
        {
            // sidebar filter
            ui.categoryFilterCombo->addItem(query.value(0).toString());

            // Password form dropdown
            QString category = query.value(0).toString();
            ui.categoryInput->addItem(category);
        }
    }
}

void HashVault::filterPasswordsByCategory(const QString& category)
{
    ui.passwordTable->setRowCount(0);

    QSqlQuery query;

    if (category == "All Categories")
    {
        query.prepare(
            "SELECT * "
            "FROM passwords "
            "WHERE user_id = ? "
            "ORDER BY id"
        );

        query.addBindValue(
            currentUserId
        );
    }
    else
    {
        query.prepare(
            "SELECT * "
            "FROM passwords "
            "WHERE user_id = ? "
            "AND category = ? "
            "ORDER BY id"
        );

        query.addBindValue(
            currentUserId
        );

        query.addBindValue(
            category
        );
    }

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