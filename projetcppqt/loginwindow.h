// loginwindow.h
#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSqlDatabase>
#include "databasemanager.h"

class DashboardWindow;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onRegisterLinkClicked();
    void onLoginLinkClicked();
    void onSubmitRegistrationClicked();
    void onRegisterClicked();

private:
    void setupUi();
    void setupDatabase();
    void connectSignals();

    QWidget* createLoginPage();
    QWidget* createRegisterPage();

    bool validateLoginForm();
    bool validateRegistrationForm();
    bool checkCredentials(const QString &email, const QString &password);
    bool createUser();

    // Database
    QSqlDatabase m_database;

    // Widgets
    QStackedWidget *m_stackedWidget;

    // Login form widgets
    QLineEdit *m_emailLoginEdit;
    QLineEdit *m_passwordLoginEdit;
    QLabel *m_loginErrorLabel;

    // Registration form widgets
    QLineEdit *m_firstNameEdit;
    QLineEdit *m_lastNameEdit;
    QSpinBox *m_ageSpinBox;
    QDoubleSpinBox *m_weightSpinBox;
    QDoubleSpinBox *m_heightSpinBox;
    QComboBox *m_fitnessLevelCombo;
    QLineEdit *m_emailRegisterEdit;
    QLineEdit *m_passwordRegisterEdit;
    QLineEdit *m_confirmPasswordEdit;
    QLabel *m_registerErrorLabel;
};

#endif // LOGINWINDOW_H
