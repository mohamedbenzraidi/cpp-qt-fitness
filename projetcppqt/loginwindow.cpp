#include "loginwindow.h"
#include "dashboardwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QPixmap>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDir>
#include <QComboBox>
#include <QGroupBox>
#include <QDateEdit>
#include <QScrollArea>
#include <QCryptographicHash>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent) {
    setFixedSize(900, 600);
    setupUi();
    setupDatabase();
    connectSignals();
}

LoginWindow::~LoginWindow() {
    // La fermeture de la base de données est gérée par DatabaseManager
}

void LoginWindow::setupUi() {
    this->setStyleSheet("background-color: #f0f2f5;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Logo en haut
    QLabel *logo = new QLabel("<b style='font-size: 28px; color: #333;'>e<span style='color:#1a73e8;'>FITNESS</span></b>");
    logo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(logo);

    // Conteneur principal
    QWidget *container = new QWidget(this);
    container->setObjectName("mainContainer");
    container->setStyleSheet(
        "#mainContainer {"
        "    background-color: white;"
        "    border-radius: 25px;"
        "    border: 1px solid #e1e4e8;"
        "}"
        );

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    // Création du widget empilé pour basculer entre connexion et inscription
    m_stackedWidget = new QStackedWidget;
    m_stackedWidget->addWidget(createLoginPage());
    m_stackedWidget->addWidget(createRegisterPage());

    containerLayout->addWidget(m_stackedWidget);

    // Ajouter au layout principal
    mainLayout->addWidget(container);

    // Ajout du footer
    QLabel *termsLabel = new QLabel("<a href='#'>Conditions d'utilisation</a> | <a href='#'>Politique de confidentialité</a>");
    termsLabel->setTextFormat(Qt::RichText);
    termsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    termsLabel->setAlignment(Qt::AlignCenter);
    termsLabel->setStyleSheet("font-size: 11px; color: #777; margin: 10px;");
    mainLayout->addWidget(termsLabel);
}

QWidget* LoginWindow::createLoginPage() {
    QWidget *loginPage = new QWidget;
    QHBoxLayout *loginLayout = new QHBoxLayout(loginPage);

    // Formulaire de connexion
    QWidget *formContainer = new QWidget;
    formContainer->setMinimumWidth(400);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(40, 40, 40, 40);
    formLayout->setSpacing(20);

    QLabel *welcomeLabel = new QLabel("Connexion à votre compte");
    welcomeLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #333; margin-bottom: 10px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Message d'erreur (invisible par défaut)
    m_loginErrorLabel = new QLabel("");
    m_loginErrorLabel->setStyleSheet("color: #d32f2f; font-size: 13px; background-color: #fdecea; padding: 8px; border-radius: 4px;");
    m_loginErrorLabel->setWordWrap(true);
    m_loginErrorLabel->setAlignment(Qt::AlignCenter);
    m_loginErrorLabel->setVisible(false);

    // Champs de saisie
    m_emailLoginEdit = new QLineEdit;
    m_emailLoginEdit->setPlaceholderText("Adresse e-mail");
    m_emailLoginEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 12px;"
        "    font-size: 14px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "    background-color: #f9f9f9;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #1a73e8;"
        "    background-color: white;"
        "}"
        );

    m_passwordLoginEdit = new QLineEdit;
    m_passwordLoginEdit->setPlaceholderText("Mot de passe");
    m_passwordLoginEdit->setEchoMode(QLineEdit::Password);
    m_passwordLoginEdit->setStyleSheet(m_emailLoginEdit->styleSheet());

    // Options supplémentaires
    QHBoxLayout *optionsLayout = new QHBoxLayout;

    QCheckBox *rememberCheck = new QCheckBox("Se souvenir de moi");
    rememberCheck->setStyleSheet("font-size: 13px; color: #555;");

    QLabel *forgotPassword = new QLabel("<a href='#' style='color: #1a73e8; text-decoration: none;'>Mot de passe oublié?</a>");
    forgotPassword->setTextFormat(Qt::RichText);
    forgotPassword->setTextInteractionFlags(Qt::TextBrowserInteraction);
    forgotPassword->setStyleSheet("font-size: 13px;");

    optionsLayout->addWidget(rememberCheck);
    optionsLayout->addStretch();
    optionsLayout->addWidget(forgotPassword);

    // Bouton de connexion
    QPushButton *loginButton = new QPushButton("Se connecter");
    loginButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #1a73e8;"
        "    color: white;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "    padding: 12px;"
        "    border-radius: 8px;"
        "    min-width: 150px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0d62d0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0b57be;"
        "}"
        );

    QHBoxLayout *loginButtonLayout = new QHBoxLayout;
    loginButtonLayout->addStretch();
    loginButtonLayout->addWidget(loginButton);
    loginButtonLayout->addStretch();

    // Lien vers l'inscription
    QLabel *registerLabel = new QLabel("Pas encore de compte? <a href='#' style='color: #1a73e8; text-decoration: none;'>Créer un compte</a>");
    registerLabel->setTextFormat(Qt::RichText);
    registerLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    registerLabel->setAlignment(Qt::AlignCenter);
    registerLabel->setStyleSheet("font-size: 14px; color: #555; margin-top: 20px;");
    connect(registerLabel, &QLabel::linkActivated, this, &LoginWindow::onRegisterLinkClicked);

    // Ajouter au layout du formulaire
    formLayout->addWidget(welcomeLabel);
    formLayout->addWidget(m_loginErrorLabel);
    formLayout->addWidget(m_emailLoginEdit);
    formLayout->addWidget(m_passwordLoginEdit);
    formLayout->addLayout(optionsLayout);
    formLayout->addSpacing(10);
    formLayout->addLayout(loginButtonLayout);
    formLayout->addWidget(registerLabel);
    formLayout->addStretch();
    loginLayout->addWidget(formContainer);

    // Image à droite
    QLabel *imageLabel = new QLabel;
    imageLabel->setMinimumWidth(400);
    QPixmap pixmap(":/images/fitness_model.png");
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap.scaled(400, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setAlignment(Qt::AlignCenter);
        loginLayout->addWidget(imageLabel);
    } else {
        // Alternative si l'image n'est pas trouvée
        QWidget *placeholderWidget = new QWidget;
        placeholderWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a73e8, stop:1 #6c92f4);");
        placeholderWidget->setMinimumWidth(400);
        loginLayout->addWidget(placeholderWidget);
    }

    return loginPage;
}

QWidget* LoginWindow::createRegisterPage() {
    QWidget *registerPage = new QWidget;

    // Utiliser QScrollArea pour permettre le défilement
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *registerLayout = new QHBoxLayout;

    // Formulaire d'inscription
    QWidget *formContainer = new QWidget;
    formContainer->setMinimumWidth(400);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(40, 40, 40, 40);
    formLayout->setSpacing(20);

    QLabel *registerTitleLabel = new QLabel("Créer un compte");
    registerTitleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #333; margin-bottom: 10px;");
    registerTitleLabel->setAlignment(Qt::AlignCenter);

    // Message d'erreur (invisible par défaut)
    m_registerErrorLabel = new QLabel("");
    m_registerErrorLabel->setStyleSheet("color: #d32f2f; font-size: 13px; background-color: #fdecea; padding: 8px; border-radius: 4px;");
    m_registerErrorLabel->setWordWrap(true);
    m_registerErrorLabel->setAlignment(Qt::AlignCenter);
    m_registerErrorLabel->setVisible(false);

    // Style commun pour les champs de saisie
    const QString inputStyle =
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
        "    padding: 10px;"
        "    font-size: 14px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "    background-color: #f9f9f9;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {"
        "    border: 1px solid #1a73e8;"
        "    background-color: white;"
        "}";

    // Informations personnelles
    QGroupBox *personalInfoGroup = new QGroupBox("Informations personnelles");
    personalInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout *personalLayout = new QFormLayout(personalInfoGroup);
    personalLayout->setSpacing(12);

    m_firstNameEdit = new QLineEdit;
    m_firstNameEdit->setPlaceholderText("Prénom");
    m_firstNameEdit->setStyleSheet(inputStyle);

    m_lastNameEdit = new QLineEdit;
    m_lastNameEdit->setPlaceholderText("Nom");
    m_lastNameEdit->setStyleSheet(inputStyle);

    m_ageSpinBox = new QSpinBox;
    m_ageSpinBox->setRange(16, 100);
    m_ageSpinBox->setValue(30);
    m_ageSpinBox->setStyleSheet(inputStyle);

    personalLayout->addRow("Prénom:", m_firstNameEdit);
    personalLayout->addRow("Nom:", m_lastNameEdit);
    personalLayout->addRow("Âge:", m_ageSpinBox);

    // Mensurations
    QGroupBox *measurementsGroup = new QGroupBox("Mensurations");
    measurementsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout *measurementsLayout = new QFormLayout(measurementsGroup);
    measurementsLayout->setSpacing(12);

    m_weightSpinBox = new QDoubleSpinBox;
    m_weightSpinBox->setRange(30.0, 200.0);
    m_weightSpinBox->setValue(70.0);
    m_weightSpinBox->setSuffix(" kg");
    m_weightSpinBox->setStyleSheet(inputStyle);

    m_heightSpinBox = new QDoubleSpinBox;
    m_heightSpinBox->setRange(100.0, 250.0);
    m_heightSpinBox->setValue(170.0);
    m_heightSpinBox->setSuffix(" cm");
    m_heightSpinBox->setStyleSheet(inputStyle);

    m_fitnessLevelCombo = new QComboBox;
    m_fitnessLevelCombo->addItems(QStringList() << "Débutant" << "Intermédiaire" << "Avancé" << "Expert");
    m_fitnessLevelCombo->setStyleSheet(inputStyle);

    measurementsLayout->addRow("Poids:", m_weightSpinBox);
    measurementsLayout->addRow("Taille:", m_heightSpinBox);
    measurementsLayout->addRow("Niveau:", m_fitnessLevelCombo);

    // Informations du compte
    QGroupBox *accountInfoGroup = new QGroupBox("Informations de connexion");
    accountInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QFormLayout *accountLayout = new QFormLayout(accountInfoGroup);
    accountLayout->setSpacing(12);

    m_emailRegisterEdit = new QLineEdit;
    m_emailRegisterEdit->setPlaceholderText("Adresse e-mail");
    m_emailRegisterEdit->setStyleSheet(inputStyle);

    m_passwordRegisterEdit = new QLineEdit;
    m_passwordRegisterEdit->setPlaceholderText("Mot de passe");
    m_passwordRegisterEdit->setEchoMode(QLineEdit::Password);
    m_passwordRegisterEdit->setStyleSheet(inputStyle);

    m_confirmPasswordEdit = new QLineEdit;
    m_confirmPasswordEdit->setPlaceholderText("Confirmer le mot de passe");
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setStyleSheet(inputStyle);

    accountLayout->addRow("Email:", m_emailRegisterEdit);
    accountLayout->addRow("Mot de passe:", m_passwordRegisterEdit);
    accountLayout->addRow("Confirmation:", m_confirmPasswordEdit);

    // Bouton d'inscription
    QPushButton *registerButton = new QPushButton("S'inscrire");
    registerButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #1a73e8;"
        "    color: white;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "    padding: 12px;"
        "    border-radius: 8px;"
        "    min-width: 150px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0d62d0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0b57be;"
        "}"
        );

    QHBoxLayout *registerButtonLayout = new QHBoxLayout;
    registerButtonLayout->addStretch();
    registerButtonLayout->addWidget(registerButton);
    registerButtonLayout->addStretch();

    // Lien vers la connexion
    QLabel *loginLabel = new QLabel("Déjà un compte? <a href='#' style='color: #1a73e8; text-decoration: none;'>Se connecter</a>");
    loginLabel->setTextFormat(Qt::RichText);
    loginLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    loginLabel->setAlignment(Qt::AlignCenter);
    loginLabel->setStyleSheet("font-size: 14px; color: #555; margin-top: 15px;");
    connect(loginLabel, &QLabel::linkActivated, this, &LoginWindow::onLoginLinkClicked);

    // Ajouter au layout du formulaire
    formLayout->addWidget(registerTitleLabel);
    formLayout->addWidget(m_registerErrorLabel);
    formLayout->addWidget(personalInfoGroup);
    formLayout->addWidget(measurementsGroup);
    formLayout->addWidget(accountInfoGroup);
    formLayout->addLayout(registerButtonLayout);
    formLayout->addWidget(loginLabel);
    formLayout->addStretch();

    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onSubmitRegistrationClicked);

    registerLayout->addWidget(formContainer);

    // Image à droite
    QLabel *imageLabel = new QLabel;
    imageLabel->setMinimumWidth(400);
    QPixmap pixmap(":/images/fitness_register.png");
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap.scaled(400, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setAlignment(Qt::AlignCenter);
        registerLayout->addWidget(imageLabel);
    } else {
        // Alternative si l'image n'est pas trouvée
        QWidget *placeholderWidget = new QWidget;
        placeholderWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a73e8, stop:1 #4caf50);");
        placeholderWidget->setMinimumWidth(400);
        registerLayout->addWidget(placeholderWidget);
    }

    scrollLayout->addLayout(registerLayout);
    scrollArea->setWidget(scrollContent);

    QHBoxLayout *mainLayout = new QHBoxLayout(registerPage);
    mainLayout->addWidget(scrollArea);

    return registerPage;
}

void LoginWindow::setupDatabase() {
    // Utiliser DatabaseManager au lieu de gérer la connexion directement
    if (!DatabaseManager::instance().isOpen()) {
        if (!DatabaseManager::instance().openDatabase()) {
            QMessageBox::critical(this, "Erreur de base de données",
                                  "Impossible d'ouvrir la base de données.");
            return;
        }
    }
}

void LoginWindow::connectSignals() {
    // Rechercher le bouton de connexion dans le premier widget du stacked widget
    QWidget *loginPage = m_stackedWidget->widget(0);
    QPushButton *loginButton = loginPage->findChild<QPushButton*>();
    if (loginButton) {
        connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    }

    // Pour le bouton d'inscription, il est connecté directement dans createRegisterPage()
}

void LoginWindow::onLoginClicked() {
    if (validateLoginForm()) {
        QString email = m_emailLoginEdit->text().trimmed();
        QString password = m_passwordLoginEdit->text();

        DatabaseManager& dbManager = DatabaseManager::instance();

        if (dbManager.checkCredentials(email, password)) {
            try {
                // Récupérer l'ID utilisateur
                int userId = dbManager.getUserId(email);

                // Récupérer les informations de l'utilisateur pour les passer au tableau de bord
                UserInfo userInfo;
                userInfo.userId = userId;
                userInfo.name = dbManager.getUserName(userId);
                userInfo.planType = dbManager.getUserPlanType(userId);

                // Récupérer les statistiques
                QSqlQuery statsQuery;
                statsQuery.prepare("SELECT workout_sessions, calories_burned, activity_minutes, exercises_done "
                                   "FROM users WHERE id = :id");
                statsQuery.bindValue(":id", userId);

                if (statsQuery.exec() && statsQuery.next()) {
                    userInfo.workoutSessions = statsQuery.value("workout_sessions").toInt();
                    userInfo.caloriesBurned = statsQuery.value("calories_burned").toInt();
                    userInfo.activityMinutes = statsQuery.value("activity_minutes").toInt();
                    userInfo.exercisesDone = statsQuery.value("exercises_done").toInt();
                } else {
                    // Valeurs par défaut si les données ne sont pas disponibles
                    userInfo.workoutSessions = 0;
                    userInfo.caloriesBurned = 0;
                    userInfo.activityMinutes = 0;
                    userInfo.exercisesDone = 0;
                }

                // Récupérer les objectifs
                QSqlQuery goalsQuery;
                goalsQuery.prepare("SELECT goal_name, progress FROM user_goals WHERE user_id = :id");
                goalsQuery.bindValue(":id", userId);

                if (goalsQuery.exec()) {
                    while (goalsQuery.next()) {
                        QString goalName = goalsQuery.value("goal_name").toString();
                        int progress = goalsQuery.value("progress").toInt();
                        userInfo.goals[goalName] = progress;
                    }
                }

                // Si aucun objectif n'a été trouvé, définir des objectifs par défaut
                if (userInfo.goals.isEmpty()) {
                    userInfo.goals["Perte de poids"] = 0;
                    userInfo.goals["Musculation"] = 0;
                    userInfo.goals["Cardio"] = 0;
                }

                // Créer et afficher le tableau de bord avec les informations utilisateur
                DashboardWindow *dashboard = new DashboardWindow(userInfo);
                dashboard->show();
                this->close();

            } catch (const std::exception& e) {
                qDebug() << "Erreur lors de la récupération des données utilisateur:" << e.what();

                // En cas d'erreur, utiliser le constructeur par défaut
                DashboardWindow *dashboard = new DashboardWindow();
                dashboard->show();
                this->close();
            }
        } else {
            m_loginErrorLabel->setText("Email ou mot de passe incorrect");
            m_loginErrorLabel->setVisible(true);
        }
    }
}

bool LoginWindow::validateLoginForm() {
    bool isValid = true;
    QString errorMsg;

    // Validation email
    QString email = m_emailLoginEdit->text().trimmed();
    QRegularExpression emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b");
    if (email.isEmpty() || !emailRegex.match(email).hasMatch()) {
        errorMsg = "Veuillez saisir une adresse e-mail valide";
        isValid = false;
    }

    // Validation mot de passe
    QString password = m_passwordLoginEdit->text();
    if (password.isEmpty()) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Veuillez saisir votre mot de passe";
        isValid = false;
    }

    // Afficher les erreurs s'il y en a
    if (!isValid) {
        m_loginErrorLabel->setText(errorMsg);
        m_loginErrorLabel->setVisible(true);
    } else {
        m_loginErrorLabel->setVisible(false);
    }

    return isValid;
}

bool LoginWindow::checkCredentials(const QString &email, const QString &password) {
    // Utiliser la méthode de DatabaseManager
    return DatabaseManager::instance().checkCredentials(email, password);
}

void LoginWindow::onRegisterLinkClicked() {
    m_stackedWidget->setCurrentIndex(1);
}

void LoginWindow::onLoginLinkClicked() {
    m_stackedWidget->setCurrentIndex(0);
}

void LoginWindow::onSubmitRegistrationClicked() {
    if (validateRegistrationForm()) {
        // Créer l'utilisateur avec gestion d'erreurs détaillée
        QString firstName = m_firstNameEdit->text().trimmed();
        QString lastName = m_lastNameEdit->text().trimmed();
        QString email = m_emailRegisterEdit->text().trimmed();
        QString password = m_passwordRegisterEdit->text();
        int age = m_ageSpinBox->value();
        double weight = m_weightSpinBox->value();
        double height = m_heightSpinBox->value();
        QString fitnessLevel = m_fitnessLevelCombo->currentText();

        // Vérifications supplémentaires avant création
        if (firstName.isEmpty() || lastName.isEmpty() || email.isEmpty()) {
            m_registerErrorLabel->setText("Tous les champs obligatoires doivent être renseignés.");
            m_registerErrorLabel->setVisible(true);
            return;
        }

        // Vérifier si l'email existe déjà AVANT d'essayer de créer l'utilisateur
        DatabaseManager& dbManager = DatabaseManager::instance();

        // Ouvrir la base de données si nécessaire
        if (!dbManager.isOpen()) {
            if (!dbManager.openDatabase()) {
                m_registerErrorLabel->setText("Erreur de connexion à la base de données.");
                m_registerErrorLabel->setVisible(true);
                return;
            }
        }

        // Vérifier si l'email existe déjà
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
        checkQuery.addBindValue(email);

        if (checkQuery.exec() && checkQuery.next()) {
            if (checkQuery.value(0).toInt() > 0) {
                m_registerErrorLabel->setText("Cette adresse e-mail est déjà utilisée. Veuillez en choisir une autre.");
                m_registerErrorLabel->setVisible(true);
                return;
            }
        } else {
            m_registerErrorLabel->setText("Erreur lors de la vérification de l'e-mail.");
            m_registerErrorLabel->setVisible(true);
            return;
        }

        // Créer l'utilisateur
        bool success = dbManager.createUser(firstName, lastName, email, password, age, weight, height, fitnessLevel);

        if (success) {
            // Effacer les champs après création réussie
            m_firstNameEdit->clear();
            m_lastNameEdit->clear();
            m_emailRegisterEdit->clear();
            m_passwordRegisterEdit->clear();
            m_confirmPasswordEdit->clear();
            m_ageSpinBox->setValue(30);
            m_weightSpinBox->setValue(70.0);
            m_heightSpinBox->setValue(170.0);
            m_fitnessLevelCombo->setCurrentIndex(0);

            // Masquer le message d'erreur
            m_registerErrorLabel->setVisible(false);

            QMessageBox::information(this, "Inscription réussie",
                                     "Votre compte a été créé avec succès. Vous pouvez maintenant vous connecter.");
            m_stackedWidget->setCurrentIndex(0);
        } else {
            // Essayer de déterminer la cause de l'erreur
            QSqlQuery retryCheck;
            retryCheck.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
            retryCheck.addBindValue(email);

            if (retryCheck.exec() && retryCheck.next() && retryCheck.value(0).toInt() > 0) {
                m_registerErrorLabel->setText("Cette adresse e-mail est déjà utilisée.");
            } else {
                m_registerErrorLabel->setText("Une erreur est survenue lors de la création du compte. "
                                              "Veuillez vérifier vos informations et réessayer.");
            }
            m_registerErrorLabel->setVisible(true);
        }
    }
}


bool LoginWindow::validateRegistrationForm() {
    bool isValid = true;
    QString errorMsg;

    // Validation des champs nom et prénom
    if (m_firstNameEdit->text().trimmed().isEmpty()) {
        errorMsg = "Le prénom est obligatoire";
        isValid = false;
    }

    if (m_lastNameEdit->text().trimmed().isEmpty()) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Le nom est obligatoire";
        isValid = false;
    }

    // Validation email
    QString email = m_emailRegisterEdit->text().trimmed();
    QRegularExpression emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b");
    if (email.isEmpty()) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "L'adresse e-mail est obligatoire";
        isValid = false;
    } else if (!emailRegex.match(email).hasMatch()) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Veuillez saisir une adresse e-mail valide";
        isValid = false;
    }

    // Validation mot de passe
    QString password = m_passwordRegisterEdit->text();
    if (password.isEmpty()) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Le mot de passe est obligatoire";
        isValid = false;
    } else if (password.length() < 6) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Le mot de passe doit contenir au moins 6 caractères";
        isValid = false;
    }

    // Vérification de la confirmation du mot de passe
    if (password != m_confirmPasswordEdit->text()) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Les mots de passe ne correspondent pas";
        isValid = false;
    }

    // Validation de l'âge
    if (m_ageSpinBox->value() < 16 || m_ageSpinBox->value() > 100) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "L'âge doit être compris entre 16 et 100 ans";
        isValid = false;
    }

    // Validation du poids
    if (m_weightSpinBox->value() < 30.0 || m_weightSpinBox->value() > 200.0) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "Le poids doit être compris entre 30 et 200 kg";
        isValid = false;
    }

    // Validation de la taille
    if (m_heightSpinBox->value() < 100.0 || m_heightSpinBox->value() > 250.0) {
        if (!errorMsg.isEmpty()) errorMsg += "<br>";
        errorMsg += "La taille doit être comprise entre 100 et 250 cm";
        isValid = false;
    }

    // Afficher les erreurs s'il y en a
    if (!isValid) {
        m_registerErrorLabel->setText(errorMsg);
        m_registerErrorLabel->setVisible(true);
    } else {
        m_registerErrorLabel->setVisible(false);
    }

    return isValid;
}
bool LoginWindow::createUser() {
    // Utiliser la méthode de DatabaseManager
    return DatabaseManager::instance().createUser(
        m_firstNameEdit->text().trimmed(),
        m_lastNameEdit->text().trimmed(),
        m_emailRegisterEdit->text().trimmed(),
        m_passwordRegisterEdit->text(),
        m_ageSpinBox->value(),
        m_weightSpinBox->value(),
        m_heightSpinBox->value(),
        m_fitnessLevelCombo->currentText()
        );
}

void LoginWindow::onRegisterClicked() {
    // Cette fonction est déjà prise en charge par onSubmitRegistrationClicked()
}
