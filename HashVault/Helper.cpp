#include "HashVault.h"


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