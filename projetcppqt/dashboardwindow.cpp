#include "dashboardwindow.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QDateTime>
#include <QMessageBox>

// Constructeur par défaut
DashboardWindow::DashboardWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Fitness App");
    resize(1600, 900);
    setMinimumSize(1600, 900);

    currentExerciseIndex = 0;
    remainingTime = 0;
    isTimerRunning = false;
    currentSet = 1;
    totalSets = 3;
    exerciseDuration = 0;
    caloriesPerMinute = 5;
    exerciseInProgress = false;

    currentUser.userId = -1; // Will be set in loadUserData
    loadUserData();

    setupUI();
    showMaximized();
}

DashboardWindow::DashboardWindow(const UserInfo& userInfo, QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Fitness App");
    resize(1600, 900);
    setMinimumSize(1600, 900);


    currentExerciseIndex = 0;
    remainingTime = 0;
    isTimerRunning = false;
    currentSet = 1;
    totalSets = 3;
    exerciseDuration = 0;
    caloriesPerMinute = 5;
    exerciseInProgress = false;

    // 1. Initialiser currentUser avec les infos de la connexion
    currentUser = userInfo;

    // 2. Charger le reste des données depuis la DB
    loadUserData();

    // 3. Construire l'interface graphique (qui utilisera les données chargées)
    setupUI();
    showMaximized();
}
void DashboardWindow::setupExercisesList() {
    exercisesList.clear();

    ExerciseInfo pushups;
    pushups.name = "Push Ups";
    pushups.description = "Great for chest, shoulders and triceps";
    pushups.imagePath = ":/images/pushups.png";
    pushups.caloriesPerMinute = 7;
    pushups.defaultDuration = 30;
    pushups.defaultSets = 3;
    pushups.muscleGroupImagePath = ":/images/chest_muscle.png";
    exercisesList.append(pushups);

    ExerciseInfo squats;
    squats.name = "Squats";
    squats.description = "Works your quads, hamstrings and glutes";
    squats.imagePath = ":/images/squats.png";
    squats.caloriesPerMinute = 8;
    squats.defaultDuration = 45;
    squats.defaultSets = 3;
    squats.muscleGroupImagePath = ":/images/legs_muscle.png";
    exercisesList.append(squats);

    ExerciseInfo planks;
    planks.name = "Plank";
    planks.description = "Strengthens your core, back and shoulders";
    planks.imagePath = ":/images/plank.png";
    planks.caloriesPerMinute = 5;
    planks.defaultDuration = 60;
    planks.defaultSets = 3;
    planks.muscleGroupImagePath = ":/images/core_muscle.png";
    exercisesList.append(planks);

    ExerciseInfo burpees;
    burpees.name = "Burpees";
    burpees.description = "Full body exercise with high intensity";
    burpees.imagePath = ":/images/burpees.png";
    burpees.caloriesPerMinute = 10;
    burpees.defaultDuration = 30;
    burpees.defaultSets = 3;
    burpees.muscleGroupImagePath = ":/images/fullbody_muscle.png";
    exercisesList.append(burpees);
}

DashboardWindow::~DashboardWindow() {
}

void DashboardWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DashboardWindow::updateTimer);

    setupExercisesList();
    setupNavSidebar();
    setupMainContent();
    setupRightSidebar();

    mainLayout->addWidget(navSidebar, 1);
    mainLayout->addWidget(mainContent, 10);
    mainLayout->addWidget(rightSidebar, 2);
    showMaximized();
}

void DashboardWindow::setupNavSidebar() {
    navSidebar = new QWidget();
    navSidebar->setObjectName("navSidebar");
    // Nouvelle couleur: bleu foncé moderne
    navSidebar->setStyleSheet("QWidget#navSidebar { background-color: #1a1d29; }");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(navSidebar);
    sidebarLayout->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    sidebarLayout->setSpacing(25);
    sidebarLayout->setContentsMargins(15, 40, 15, 20);

    // Logo section
    QLabel *logoLabel = new QLabel();
    QPixmap logoPixmap(":/icons/logo.png");
    if (logoPixmap.isNull()) {
        logoLabel->setText("FITNESS");
        logoLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #64ffda; text-align: center;");
    } else {
        logoLabel->setPixmap(logoPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(logoLabel);
    sidebarLayout->addSpacing(30);

    QStringList menuItems = {"Exercice", "P.alimentaire", "Habitudes", "Eau"};
    QStringList iconPaths = {":/icons/exercise.png", ":/icons/meal.png", ":/icons/habits.png", ":/icons/water.png"};

    for (int i = 0; i < menuItems.size(); ++i) {
        QPushButton *menuItem = new QPushButton();
        menuItem->setCursor(Qt::PointingHandCursor);
        menuItem->setMinimumHeight(80);
        menuItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // Layout vertical pour icône + texte
        QVBoxLayout *itemLayout = new QVBoxLayout(menuItem);
        itemLayout->setAlignment(Qt::AlignCenter);
        itemLayout->setSpacing(8);
        itemLayout->setContentsMargins(10, 15, 10, 15);

        // Conteneur pour l'icône
        QLabel *iconLabel = new QLabel();
        QPixmap iconPixmap(iconPaths[i]);
        if (!iconPixmap.isNull()) {
            iconLabel->setPixmap(iconPixmap.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // Icons de fallback
            QStringList fallbackIcons = {"", "", "", ""};
            iconLabel->setText(fallbackIcons[i]);
            iconLabel->setStyleSheet("font-size: 24px; padding:10px 0px;");
        }
        iconLabel->setAlignment(Qt::AlignCenter);

        // Label pour le texte
        QLabel *textLabel = new QLabel(menuItems[i]);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setWordWrap(true);
        textLabel->setStyleSheet(
            "color: #e0e6ed; "
            "font-size: 12px; "
            "font-weight: 500; "
            "background: transparent; "
            "border: none;"
            );

        itemLayout->addWidget(iconLabel);
        itemLayout->addWidget(textLabel);

        // Style du bouton
        if (i == 0) {
            // Premier élément sélectionné par défaut
            menuItem->setStyleSheet(
                "QPushButton {"
                "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #64ffda, stop:1 #1de9b6);"
                "border: none;"
                "border-radius: 12px;"
                "padding: 8px;"
                "}"
                "QPushButton:hover {"
                "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4fd3b8, stop:1 #1dd1a1);"
                "}"
                );
            textLabel->setStyleSheet(
                "color: #1a1d29; "
                "font-size: 12px; "
                "font-weight: 600; "
                "background: transparent;"
                "padding:0px;"
                );
        } else {
            menuItem->setStyleSheet(
                "QPushButton {"
                "background-color: transparent;"
                "border: none;"
                "border-radius: 12px;"
                "padding: 8px;"
                "padding: 0px;"
                "}"
                "QPushButton:hover {"
                "background-color: rgba(100, 255, 218, 0.1);"
                "}"
                "QPushButton:pressed {"
                "background-color: rgba(100, 255, 218, 0.2);"
                "}"
                );
        }

        connect(menuItem, &QPushButton::clicked, [this, i]() {
            this->switchToView(i);
        });

        sidebarLayout->addWidget(menuItem);
    }

    sidebarLayout->addStretch();
}

void DashboardWindow::setupMainContent() {
    mainContent = new QStackedWidget();
    exerciseView = new QWidget();
    mealPlanView = new MealPlanView(currentUser.userId, this);
    habitsView = new HabitsView(currentUser.userId, this);
    waterView = new WaterWidget(currentUser.userId, this);

    setupExerciseView();

    habitsView->setStyleSheet("background-color: #f8f9fa;");
    waterView->setStyleSheet("background-color: #f8f9fa;");

    mainContent->addWidget(exerciseView);
    mainContent->addWidget(mealPlanView);
    mainContent->addWidget(habitsView);
    mainContent->addWidget(waterView);

    mainContent->setCurrentIndex(0);
}

void DashboardWindow::setupRightSidebar() {
    rightSidebar = new QWidget();
    rightSidebar->setObjectName("rightSidebar");
    rightSidebar->setStyleSheet("QWidget#rightSidebar { background-color: #e9ecef; }");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(rightSidebar);
    sidebarLayout->setSpacing(20);
    sidebarLayout->setContentsMargins(15, 30, 15, 30);

    QWidget *profileWidget = new QWidget();
    profileWidget->setStyleSheet("background-color: white; border-radius: 15px; padding: 15px;");
    QVBoxLayout *profileLayout = new QVBoxLayout();
    profileLayout->setAlignment(Qt::AlignCenter);

    QLabel *profileLabel = new QLabel();
    QPixmap profilePixmap(":/icons/profile.png");
    if (!profilePixmap.isNull()) {
        profileLabel->setPixmap(profilePixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        profileLabel->setText("👤");
        profileLabel->setStyleSheet("font-size: 32px; color: #4361ee;");
    }
    profileLabel->setAlignment(Qt::AlignCenter);

    QLabel *nameLabel = new QLabel(currentUser.name);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2b2d42;");
    nameLabel->setAlignment(Qt::AlignCenter);

    QLabel *planLabel = new QLabel(QString("Plan: %1").arg(currentUser.planType));
    planLabel->setStyleSheet("font-size: 14px; color: #8d99ae;");
    planLabel->setAlignment(Qt::AlignCenter);

    profileLayout->addWidget(profileLabel);
    profileLayout->addWidget(nameLabel);
    profileLayout->addWidget(planLabel);
    profileWidget->setLayout(profileLayout);
    sidebarLayout->addWidget(profileWidget);

    QWidget *statsWidget = new QWidget();
    statsWidget->setStyleSheet("background-color: white; border-radius: 15px; padding: 15px;");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsWidget);

    QLabel *statsTitle = new QLabel("Vos statistiques");
    statsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2b2d42;");
    statsLayout->addWidget(statsTitle);

    QStringList statNames = {"Séances d'entraînement", "Calories brûlées", "Minutes d'activité"};
    QStringList statValues = {
        QString("%1 cette semaine").arg(currentUser.workoutSessions),
        QString("%1 kcal").arg(currentUser.caloriesBurned),
        QString("%1 min").arg(currentUser.activityMinutes)
    };

    for (int i = 0; i < statNames.size(); ++i) {
        QWidget *statItem = new QWidget();
        QHBoxLayout *statLayout = new QHBoxLayout(statItem);
        statLayout->setContentsMargins(0, 8, 0, 8);

        QLabel *statNameLabel = new QLabel(statNames[i]);
        statNameLabel->setStyleSheet("color: #8d99ae; padding:10px;");

        QLabel *statValueLabel = new QLabel(statValues[i]);
        statValueLabel->setStyleSheet("font-weight: bold; color: #2b2d42;padding:10px;");

        statLayout->addWidget(statNameLabel);
        statLayout->addStretch();
        statLayout->addWidget(statValueLabel);
        statsLayout->addWidget(statItem);
    }

    sidebarLayout->addWidget(statsWidget);

    QWidget *goalsWidget = new QWidget();
    goalsWidget->setObjectName("goalsWidget");
    goalsWidget->setStyleSheet("background-color: white; border-radius: 15px; padding: 15px;");
    QVBoxLayout *goalsLayout = new QVBoxLayout(goalsWidget);

    QLabel *goalsTitle = new QLabel("Vos objectifs");
    goalsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2b2d42;");
    goalsLayout->addWidget(goalsTitle);

    QStringList goalNames = currentUser.goals.keys();

    for (int i = 0; i < goalNames.size(); ++i) {
        QWidget *goalItem = new QWidget();
        QVBoxLayout *goalLayout = new QVBoxLayout(goalItem);
        goalLayout->setContentsMargins(0, 8, 0, 8);

        QHBoxLayout *labelLayout = new QHBoxLayout();
        QLabel *goalNameLabel = new QLabel(goalNames[i]);
        goalNameLabel->setStyleSheet("color: #8d99ae;");

        int goalValue = currentUser.goals[goalNames[i]];
        QLabel *goalValueLabel = new QLabel(QString("%1%").arg(goalValue));
        goalValueLabel->setStyleSheet("font-weight: bold; color: #2b2d42;");

        labelLayout->addWidget(goalNameLabel);
        labelLayout->addStretch();
        labelLayout->addWidget(goalValueLabel);

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(goalValue);
        progressBar->setStyleSheet(
            "QProgressBar { background-color: #e9ecef; border-radius: 5px; height: 10px; } "
            "QProgressBar::chunk { background-color: #4cc9f0; border-radius: 5px; }"
            );

        goalLayout->addLayout(labelLayout);
        goalLayout->addWidget(progressBar);
        goalsLayout->addWidget(goalItem);
    }

    sidebarLayout->addWidget(goalsWidget);
    sidebarLayout->addStretch();
}

void DashboardWindow::setupExerciseView() {
    exerciseView->setObjectName("exerciseView");
    exerciseView->setStyleSheet("QWidget#exerciseView { background-color: transparent; }");

    QVBoxLayout *contentLayout = new QVBoxLayout(exerciseView);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(20, 0, 20, 20);

    QLabel *mainTitle = new QLabel("Shape your ideal body");
    mainTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #2b2d42;");
    contentLayout->addWidget(mainTitle);

    QWidget *timerSection = createTimerSection();
    timerSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    contentLayout->addWidget(timerSection);

    QWidget *muscleSection = createMuscleSection();
    muscleSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    contentLayout->addWidget(muscleSection, 1);

    QWidget *dataCardsSection = createDataCardsSection();
    dataCardsSection->setObjectName("dataCardsSection");
    dataCardsSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    contentLayout->addWidget(dataCardsSection);

    contentLayout->addStretch(0);
}

void DashboardWindow::setExerciseType(const QString& exerciseName, int caloriesRate) {
    activityLabel->setText(exerciseName);
    caloriesPerMinute = caloriesRate;
}

void DashboardWindow::startExercise(const QString& exerciseName, int sets, int duration) {
    activityLabel = exerciseNameLabel;
    totalSets = sets;
    currentSet = 1;
    remainingTime = duration;
    caloriesPerMinute = 6;

    for (const ExerciseInfo& exercise : exercisesList) {
        if (exercise.name == exerciseName) {
            caloriesPerMinute = exercise.caloriesPerMinute;
            if (muscleMapLabel) {
                QPixmap muscleMapPixmap(exercise.muscleGroupImagePath);
                if (!muscleMapPixmap.isNull()) {
                    muscleMapLabel->setPixmap(muscleMapPixmap.scaled(muscleMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
            break;
        }
    }

    exerciseInProgress = false;
    exerciseDuration = 0;
    setCountLabel->setText(QString("Série %1/%2").arg(currentSet).arg(totalSets));
    updateTimerDisplay();
    startTimer();
}

QWidget* DashboardWindow::createTimerSection() {
    QWidget *timerSection = new QWidget();
    QHBoxLayout *timerLayout = new QHBoxLayout(timerSection);
    timerLayout->setSpacing(10);
    timerLayout->setContentsMargins(0, 0, 0, 0);

    // Création du widget carousel pour les exercices
    exerciseCarouselWidget = new QWidget();
    exerciseCarouselWidget->setStyleSheet(
        "background-color: white; "
        "border-radius: 12px; "
        "box-shadow: 0 4px 12px rgba(0,0,0,0.1);"
        );
    QVBoxLayout *carouselLayout = new QVBoxLayout(exerciseCarouselWidget);
    carouselLayout->setSpacing(8);
    carouselLayout->setContentsMargins(10, 10, 10, 10);

    // Widget de la carte d'exercice
    exerciseCardWidget = new QWidget();
    exerciseCardWidget->setFixedHeight(320);
    exerciseCardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    exerciseCardWidget->setStyleSheet(
        "background-color: white; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 10px;"
        );

    QHBoxLayout *cardLayout = new QHBoxLayout(exerciseCardWidget);
    cardLayout->setContentsMargins(8, 8, 8, 8);
    cardLayout->setSpacing(10);

    // Conteneur pour l'image
    QWidget *imageContainer = new QWidget();
    imageContainer->setFixedSize(200, 300);
    imageContainer->setStyleSheet(
        "background-color: #f5f5f5; "
        "border-radius: 8px; "
        "border: 2px solid #4cc9f0;"
        );
    QVBoxLayout *imageLayout = new QVBoxLayout(imageContainer);
    imageLayout->setContentsMargins(4, 4, 4, 4);

    // Label pour l'image
    exerciseImageLabel = new QLabel();
    exerciseImageLabel->setFixedSize(192, 292);
    exerciseImageLabel->setScaledContents(true);
    imageLayout->addWidget(exerciseImageLabel);

    // Conteneur pour les informations textuelles
    QWidget *textContainer = new QWidget();
    textContainer->setStyleSheet(
        "background-color: rgba(255, 255, 255, 0.9); "
        "border-radius: 8px;"
        );
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setSpacing(8);
    textLayout->setContentsMargins(12, 12, 12, 12);

    // Titre de l'exercice
    exerciseNameLabel = new QLabel();
    exerciseNameLabel->setStyleSheet(
        "font-size: 22px; "
        "font-weight: bold; "
        "color: #2b2d42;"
        );
    exerciseNameLabel->setAlignment(Qt::AlignLeft);
    textLayout->addWidget(exerciseNameLabel);

    // Description de l'exercice
    exerciseDescLabel = new QLabel();
    exerciseDescLabel->setStyleSheet(
        "font-size: 13px; "
        "color: #555555;"
        );
    exerciseDescLabel->setWordWrap(true);
    textLayout->addWidget(exerciseDescLabel);

    // Boutons de navigation
    QHBoxLayout *navButtonsLayout = new QHBoxLayout();
    navButtonsLayout->setSpacing(8);

    prevExerciseButton = new QPushButton("◄");
    prevExerciseButton->setFixedSize(28, 28);
    prevExerciseButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4cc9f0; "
        "color: white; "
        "border: none; "
        "border-radius: 14px; "
        "font-size: 14px; "
        "}"
        "QPushButton:hover { "
        "background-color: #4361ee; "
        "}"
        );
    connect(prevExerciseButton, &QPushButton::clicked, this, &DashboardWindow::previousExercise);

    nextExerciseButton = new QPushButton("►");
    nextExerciseButton->setFixedSize(28, 28);
    nextExerciseButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4cc9f0; "
        "color: white; "
        "border: none; "
        "border-radius: 14px; "
        "font-size: 14px; "
        "}"
        "QPushButton:hover { "
        "background-color: #4361ee; "
        "}"
        );
    connect(nextExerciseButton, &QPushButton::clicked, this, &DashboardWindow::nextExercise);

    navButtonsLayout->addWidget(prevExerciseButton);
    navButtonsLayout->addWidget(nextExerciseButton);
    navButtonsLayout->addStretch();

    textLayout->addLayout(navButtonsLayout);

    // Bouton pour sélectionner l'exercice
    QPushButton *selectButton = new QPushButton("Select Exercise");
    selectButton->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4cc9f0, stop:1 #4361ee); "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 6px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "}"
        "QPushButton:hover { "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4361ee, stop:1 #4cc9f0); "
        "}"
        );
    connect(selectButton, &QPushButton::clicked, this, &DashboardWindow::selectCurrentExercise);
    textLayout->addWidget(selectButton, 0, Qt::AlignLeft);

    // Afficher l'info de série
    setCountLabel = new QLabel(QString("Série %1/%2").arg(currentSet).arg(totalSets));
    setCountLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #8d99ae; "
        "background: transparent;"
        );
    textLayout->addWidget(setCountLabel, 0, Qt::AlignLeft);

    textLayout->addStretch();

    // Ajout des conteneurs au layout de la carte
    cardLayout->addWidget(imageContainer);
    cardLayout->addWidget(textContainer, 1);

    // Ajout de la carte au carousel
    carouselLayout->addWidget(exerciseCardWidget);
    carouselLayout->addStretch();

    // Timer display
    QWidget *timerWidget = new QWidget();
    timerWidget->setStyleSheet(
        "background-color: white; "
        "border-radius: 12px; "
        "box-shadow: 0 2px 6px rgba(0,0,0,0.1);"
        );
    timerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QVBoxLayout *timerDisplayLayout = new QVBoxLayout(timerWidget);
    timerDisplayLayout->setSpacing(8);
    timerDisplayLayout->setContentsMargins(10, 10, 10, 10);

    timerLabel = new QLabel("Temps restant");
    timerLabel->setStyleSheet("font-size: 14px; color: #8d99ae;");
    timerDisplayLayout->addWidget(timerLabel);

    timerDisplayValue = new QLabel("00:30");
    timerDisplayValue->setStyleSheet("font-size: 28px; font-weight: bold; color: #2b2d42;");
    timerDisplayLayout->addWidget(timerDisplayValue);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(8);

    QPushButton *startButton = new QPushButton("Start");
    startButton->setStyleSheet("background-color: #4cc9f0; color: white; border: none; padding: 6px 12px; border-radius: 5px;");
    connect(startButton, &QPushButton::clicked, this, &DashboardWindow::startTimer);

    QPushButton *pauseButton = new QPushButton("Pause");
    pauseButton->setStyleSheet("background-color: #f72585; color: white; border: none; padding: 6px 12px; border-radius: 5px;");
    connect(pauseButton, &QPushButton::clicked, this, &DashboardWindow::pauseTimer);

    QPushButton *resetButton = new QPushButton("Reset");
    resetButton->setStyleSheet("background-color: #8d99ae; color: white; border: none; padding: 6px 12px; border-radius: 5px;");
    connect(resetButton, &QPushButton::clicked, this, &DashboardWindow::resetTimer);

    QPushButton *finishButton = new QPushButton("Terminer");
    finishButton->setStyleSheet("background-color: #4361ee; color: white; border: none; padding: 6px 12px; border-radius: 5px;");
    connect(finishButton, &QPushButton::clicked, this, &DashboardWindow::finishExercise);

    controlsLayout->addWidget(startButton);
    controlsLayout->addWidget(pauseButton);
    controlsLayout->addWidget(resetButton);
    controlsLayout->addWidget(finishButton);

    timerDisplayLayout->addLayout(controlsLayout);
    timerLayout->addWidget(exerciseCarouselWidget, 1);
    timerLayout->addWidget(timerWidget, 0);

    updateExerciseDisplay();
    return timerSection;
}

void DashboardWindow::updateExerciseDisplay() {
    if (exercisesList.isEmpty()) {
        return;
    }

    const ExerciseInfo& currentExercise = exercisesList[currentExerciseIndex];

    QPixmap exercisePixmap(currentExercise.imagePath);
    if (!exercisePixmap.isNull()) {
        exerciseImageLabel->setPixmap(exercisePixmap.scaled(
            192, 292, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        exerciseImageLabel->setText("Image indisponible");
        exerciseImageLabel->setStyleSheet("font-size: 14px; color: #8d99ae; background-color: #f5f5f5;");
    }

    exerciseNameLabel->setText(currentExercise.name);
    exerciseDescLabel->setText(currentExercise.description);

    if (muscleMapLabel) {
        QPixmap muscleMapPixmap(currentExercise.muscleGroupImagePath);
        if (!muscleMapPixmap.isNull()) {
            muscleMapLabel->setPixmap(muscleMapPixmap.scaled(
                muscleMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            muscleMapLabel->setText("Muscle Map Placeholder");
            muscleMapLabel->setStyleSheet("font-size: 20px; color: #8d99ae; background-color: #f5f5f5; "
                                          "border-radius: 0px; padding: 0px;");
        }
    }

    setCountLabel->setText(QString("Série %1/%2").arg(currentSet).arg(totalSets));
    exerciseCardWidget->update();
    exerciseCardWidget->adjustSize();
}

void DashboardWindow::nextExercise() {
    if (exercisesList.isEmpty()) {
        return;
    }

    currentExerciseIndex = (currentExerciseIndex + 1) % exercisesList.size();
    updateExerciseDisplay();
}

void DashboardWindow::previousExercise() {
    if (exercisesList.isEmpty()) {
        return;
    }

    currentExerciseIndex = (currentExerciseIndex - 1 + exercisesList.size()) % exercisesList.size();
    updateExerciseDisplay();
}

void DashboardWindow::selectCurrentExercise() {
    if (exercisesList.isEmpty()) {
        return;
    }

    const ExerciseInfo& exercise = exercisesList[currentExerciseIndex];
    startExercise(exercise.name, exercise.defaultSets, exercise.defaultDuration);
}

QWidget* DashboardWindow::createMuscleSection() {
    QWidget *muscleSection = new QWidget();
    muscleSection->setStyleSheet("background-color: white; border-radius: 12px;");
    muscleSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *muscleLayout = new QVBoxLayout(muscleSection);
    muscleLayout->setSpacing(6);
    muscleLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *sectionTitle = new QLabel("Target Muscle Groups");
    sectionTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2b2d42; margin: 0; padding: 0;");
    muscleLayout->addWidget(sectionTitle);

    QLabel *instructionLabel = new QLabel("Select the muscle groups you want to focus on");
    instructionLabel->setStyleSheet("font-size: 12px; color: #8d99ae; margin: 0; padding: 0;");
    muscleLayout->addWidget(instructionLabel);

    muscleMapLabel = new QLabel();
    muscleMapLabel->setAlignment(Qt::AlignCenter);
    muscleMapLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    muscleMapLabel->setScaledContents(true);

    muscleLayout->addWidget(muscleMapLabel, 1);

    QPixmap muscleMapPixmap;
    if (!exercisesList.isEmpty()) {
        muscleMapPixmap = QPixmap(exercisesList[currentExerciseIndex].muscleGroupImagePath);
    } else {
        muscleMapPixmap = QPixmap(":/images/muscle_map.png");
    }

    if (!muscleMapPixmap.isNull()) {
        muscleMapLabel->setPixmap(muscleMapPixmap.scaled(
            muscleMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        muscleMapLabel->setText("Muscle Map Placeholder");
        muscleMapLabel->setStyleSheet("font-size: 20px; color: #8d99ae; background-color: #f5f5f5; "
                                      "border-radius: 0px; padding: 0px;");
    }

    return muscleSection;
}

QWidget* DashboardWindow::createDataCardsSection() {
    QWidget *dataCardsSection = new QWidget();
    QHBoxLayout *cardsLayout = new QHBoxLayout(dataCardsSection);
    cardsLayout->setSpacing(15);

    QStringList cardTitles = {"Exercises Done", "Calories Burned", "Activity Minutes"};
    QStringList cardValues = {
        QString::number(currentUser.exercisesDone),
        QString::number(currentUser.caloriesBurned),
        QString::number(currentUser.activityMinutes)
    };
    QStringList cardIcons = {":/icons/exercise_done.png", ":/icons/calories.png", ":/icons/time.png"};
    QStringList iconFallbacks = {"🏋️", "🔥", "⏱️"};
    QStringList cardColors = {"#4cc9f0", "#f72585", "#4361ee"};

    for (int i = 0; i < cardTitles.size(); ++i) {
        QWidget *card = new QWidget();
        card->setStyleSheet(QString("background-color: %1; border-radius: 10px; padding: 10px;").arg(cardColors[i]));
        card->setMaximumWidth(200);
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(4);

        QLabel *iconLabel = new QLabel();
        QPixmap iconPixmap(cardIcons[i]);
        if (!iconPixmap.isNull()) {
            iconLabel->setPixmap(iconPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            iconLabel->setText(iconFallbacks[i]);
            iconLabel->setStyleSheet("font-size: 20px; color: white;");
        }

        QLabel *valueLabel = new QLabel(cardValues[i]);
        valueLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

        QLabel *titleLabel = new QLabel(cardTitles[i]);
        titleLabel->setStyleSheet("font-size: 12px; color: white;");

        cardLayout->addWidget(iconLabel);
        cardLayout->addWidget(valueLabel);
        cardLayout->addWidget(titleLabel);
        cardsLayout->addWidget(card);
    }

    return dataCardsSection;
}

void DashboardWindow::switchToView(int viewIndex) {
    if (viewIndex >= 0 && viewIndex < mainContent->count()) {
        mainContent->setCurrentIndex(viewIndex);

        // Mise à jour du style des éléments de menu
        QVBoxLayout *sidebarLayout = qobject_cast<QVBoxLayout*>(navSidebar->layout());
        if (!sidebarLayout) return;

        QStringList menuItems = {"Exercice", "Plan alimentaire", "Habitudes", "Eau"};

        // Commencer à partir de l'index 2 (après logo et spacing)
        int menuStartIndex = 2;

        for (int i = 0; i < menuItems.size(); ++i) {
            int layoutIndex = menuStartIndex + i;
            if (layoutIndex < sidebarLayout->count()) {
                QLayoutItem *item = sidebarLayout->itemAt(layoutIndex);
                if (item && item->widget()) {
                    QPushButton *menuItem = qobject_cast<QPushButton*>(item->widget());
                    if (menuItem) {
                        // Trouver le label de texte dans le bouton
                        QVBoxLayout *itemLayout = qobject_cast<QVBoxLayout*>(menuItem->layout());
                        if (itemLayout && itemLayout->count() >= 2) {
                            QLabel *textLabel = qobject_cast<QLabel*>(itemLayout->itemAt(1)->widget());

                            if (i == viewIndex) {
                                // Style pour l'élément sélectionné
                                menuItem->setStyleSheet(
                                    "QPushButton {"
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #64ffda, stop:1 #1de9b6);"
                                    "border: none;"
                                    "border-radius: 12px;"
                                     "padding: 30px 0px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4fd3b8, stop:1 #1dd1a1);"
                                    "}"
                                    );
                                if (textLabel) {
                                    textLabel->setStyleSheet(
                                        "color: #1a1d29; "
                                        "font-size: 12px; "
                                        "font-weight: 600; "
                                         "padding: 30px 0px;"
                                        "background: transparent;"
                                        );
                                }
                            } else {
                                // Style pour les éléments non sélectionnés
                                menuItem->setStyleSheet(
                                    "QPushButton {"
                                    "background-color: transparent;"
                                    "border: none;"
                                    "border-radius: 12px;"
                                      "padding: 30px 0px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "background-color: rgba(100, 255, 218, 0.1);"
                                    "}"
                                    "QPushButton:pressed {"
                                    "background-color: rgba(100, 255, 218, 0.2);"
                                    "}"
                                    );
                                if (textLabel) {
                                    textLabel->setStyleSheet(
                                        "color: #e0e6ed; "
                                        "font-size: 12px; "
                                        "font-weight: 500; "
                                        "padding: 30px 0px;"
                                        "background: transparent;"
                                        );
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void DashboardWindow::startTimer() {
    if (!isTimerRunning) {
        if (remainingTime <= 0) {
            remainingTime = 30;
        }
        timer->start(1000);
        isTimerRunning = true;

        if (!exerciseInProgress) {
            exerciseInProgress = true;
            exerciseStartTime = QTime::currentTime();
        }

        updateTimerDisplay();
    }
}

void DashboardWindow::pauseTimer() {
    if (isTimerRunning) {
        timer->stop();
        isTimerRunning = false;

        if (exerciseInProgress) {
            exerciseDuration += exerciseStartTime.secsTo(QTime::currentTime());
        }
    }
}

void DashboardWindow::resetTimer() {
    timer->stop();
    isTimerRunning = false;
    remainingTime = 30;
    exerciseInProgress = false;
    exerciseDuration = 0;
    updateTimerDisplay();
}

void DashboardWindow::updateTimer() {
    if (remainingTime > 0) {
        remainingTime--;
        updateTimerDisplay();
    } else {
        timer->stop();
        isTimerRunning = false;

        if (exerciseInProgress) {
            QTime currentTime = QTime::currentTime();
            int elapsed = exerciseStartTime.secsTo(currentTime);
            exerciseDuration += elapsed;

            int seriesCalories = (elapsed / 60.0) * caloriesPerMinute;
            currentUser.caloriesBurned += seriesCalories;
            currentUser.activityMinutes += elapsed / 60;
            updateStatisticsDisplay();
        }

        if (currentSet < totalSets) {
            currentSet++;
            setCountLabel->setText(QString("Série %1/%2").arg(currentSet).arg(totalSets));
            remainingTime = 30;

            QMessageBox::information(this, "Série terminée",
                                     QString("Série %1/%2 terminée! Préparez-vous pour la prochaine série.")
                                         .arg(currentSet-1).arg(totalSets));
            startTimer();
        } else {
            updateUserStats();
            QMessageBox::information(this, "Exercice terminé",
                                     QString("Félicitations! Vous avez terminé toutes les séries.\n"
                                             "Temps d'activité: %1 minutes\n"
                                             "Calories brûlées: %2")
                                         .arg(exerciseDuration / 60)
                                         .arg(exerciseDuration / 60 * caloriesPerMinute));

            currentSet = 1;
            setCountLabel->setText(QString("Série %1/%2").arg(currentSet).arg(totalSets));
            exerciseInProgress = false;
            exerciseDuration = 0;
            updateStatisticsDisplay();
        }
    }
}

void DashboardWindow::updateUserStats() {
    if (exerciseInProgress) {
        exerciseDuration += exerciseStartTime.secsTo(QTime::currentTime());
        exerciseStartTime = QTime::currentTime();
    }

    int caloriesBurned = calculateCaloriesBurned();
    currentUser.workoutSessions++;
    currentUser.caloriesBurned += caloriesBurned;
    currentUser.activityMinutes += exerciseDuration / 60;
    currentUser.exercisesDone++;

    QString currentExerciseName = exercisesList[currentExerciseIndex].name;
    if (currentExerciseName.contains("Push", Qt::CaseInsensitive) ||
        currentExerciseName.contains("Plank", Qt::CaseInsensitive)) {
        currentUser.goals["Musculation"] = qMin(100, currentUser.goals["Musculation"] + 10);
    }
    else if (currentExerciseName.contains("Squat", Qt::CaseInsensitive) ||
             currentExerciseName.contains("Burpee", Qt::CaseInsensitive)) {
        currentUser.goals["Cardio"] = qMin(100, currentUser.goals["Cardio"] + 15);
    }

    currentUser.goals["Perte de poids"] = qMin(100, currentUser.goals["Perte de poids"] + 5);
    saveUserStats();
    animateStats();
}

void DashboardWindow::animateStats() {
    QWidget* dataCardsSection = nullptr;
    for (int i = 0; i < exerciseView->layout()->count(); ++i) {
        QWidget* widget = exerciseView->layout()->itemAt(i)->widget();
        if (widget && widget->objectName() == "dataCardsSection") {
            dataCardsSection = widget;
            break;
        }
    }

    if (dataCardsSection) {
        QHBoxLayout* cardsLayout = qobject_cast<QHBoxLayout*>(dataCardsSection->layout());
        if (cardsLayout) {
            for (int i = 0; i < cardsLayout->count(); ++i) {
                QWidget* card = cardsLayout->itemAt(i)->widget();
                QString originalStyle = card->styleSheet();
                card->setStyleSheet(originalStyle + "");
                QTimer::singleShot(1000, [card, originalStyle]() {
                    card->setStyleSheet(originalStyle);
                });
            }
        }
    }

    QWidget* goalsWidget = nullptr;
    for (int i = 0; i < rightSidebar->layout()->count(); ++i) {
        QWidget* widget = rightSidebar->layout()->itemAt(i)->widget();
        if (widget && widget->objectName() == "goalsWidget") {
            goalsWidget = widget;
            QString originalStyle = goalsWidget->styleSheet();
            goalsWidget->setStyleSheet(originalStyle + "");
            QTimer::singleShot(1000, [goalsWidget, originalStyle]() {
                goalsWidget->setStyleSheet(originalStyle);
            });
            break;
        }
    }
}

int DashboardWindow::calculateCaloriesBurned() {
    return (exerciseDuration / 60) * caloriesPerMinute;
}

void DashboardWindow::saveUserStats()
{
    // Ne rien sauvegarder si l'utilisateur n'est pas valide
    if (currentUser.userId == -1) {
        return;
    }

    // Sauvegarder les statistiques
    DatabaseManager::instance().updateUserStats(currentUser.userId, currentUser.workoutSessions,
                                                currentUser.caloriesBurned, currentUser.activityMinutes,
                                                currentUser.exercisesDone);

    // Sauvegarder les objectifs
    DatabaseManager::instance().saveUserGoals(currentUser.userId, currentUser.goals);
}

void DashboardWindow::updateTimerDisplay() {
    int minutes = remainingTime / 60;
    int seconds = remainingTime % 60;

    timerDisplayValue->setText(QString("%1:%2")
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0')));
}

void DashboardWindow::updateStatisticsDisplay() {
    QWidget* dataCardsSection = nullptr;
    for (int i = 0; i < exerciseView->layout()->count(); ++i) {
        QWidget* widget = exerciseView->layout()->itemAt(i)->widget();
        if (widget && widget->objectName() == "dataCardsSection") {
            dataCardsSection = widget;
            break;
        }
    }

    if (dataCardsSection) {
        QHBoxLayout* cardsLayout = qobject_cast<QHBoxLayout*>(dataCardsSection->layout());
        if (cardsLayout) {
            if (cardsLayout->count() > 0) {
                QWidget* exerciseCard = cardsLayout->itemAt(0)->widget();
                QLabel* exerciseLabel = qobject_cast<QLabel*>(exerciseCard->layout()->itemAt(1)->widget());
                if (exerciseLabel) {
                    exerciseLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f72585;");
                    exerciseLabel->setText(QString::number(currentUser.exercisesDone));
                    QTimer::singleShot(300, [exerciseLabel]() {
                        exerciseLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
                    });
                }
            }

            if (cardsLayout->count() > 1) {
                QWidget* caloriesCard = cardsLayout->itemAt(1)->widget();
                QLabel* calorieLabel = qobject_cast<QLabel*>(caloriesCard->layout()->itemAt(1)->widget());
                if (calorieLabel) {
                    calorieLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f72585;");
                    calorieLabel->setText(QString::number(currentUser.caloriesBurned));
                    QTimer::singleShot(300, [calorieLabel]() {
                        calorieLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
                    });
                }
            }

            if (cardsLayout->count() > 2) {
                QWidget* minutesCard = cardsLayout->itemAt(2)->widget();
                QLabel* minutesLabel = qobject_cast<QLabel*>(minutesCard->layout()->itemAt(1)->widget());
                if (minutesLabel) {
                    minutesLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f72585;");
                    minutesLabel->setText(QString::number(currentUser.activityMinutes));
                    QTimer::singleShot(300, [minutesLabel]() {
                        minutesLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
                    });
                }
            }
        }
    }

    for (int i = 0; i < rightSidebar->layout()->count(); ++i) {
        QLayoutItem* item = rightSidebar->layout()->itemAt(i);
        if (!item || !item->widget()) continue;

        QWidget* widget = item->widget();
        QVBoxLayout* widgetLayout = qobject_cast<QVBoxLayout*>(widget->layout());
        if (!widgetLayout) continue;

        for (int j = 0; j < widgetLayout->count(); ++j) {
            QLayoutItem* titleItem = widgetLayout->itemAt(j);
            if (!titleItem || !titleItem->widget()) continue;

            QLabel* label = qobject_cast<QLabel*>(titleItem->widget());
            if (label && label->text() == "Vos statistiques") {
                for (int k = j + 1; k < widgetLayout->count(); ++k) {
                    QLayoutItem* statItem = widgetLayout->itemAt(k);
                    if (!statItem || !statItem->widget()) continue;

                    QWidget* statWidget = statItem->widget();
                    QHBoxLayout* statLayout = qobject_cast<QHBoxLayout*>(statWidget->layout());
                    if (!statLayout || statLayout->count() < 2) continue;

                    QLabel* statNameLabel = qobject_cast<QLabel*>(statLayout->itemAt(0)->widget());
                    QLabel* statValueLabel = qobject_cast<QLabel*>(statLayout->itemAt(statLayout->count() - 1)->widget());
                    if (!statNameLabel || !statValueLabel) continue;

                    QString statName = statNameLabel->text();
                    QString newValue;

                    if (statName == "Séances d'entraînement") {
                        newValue = QString("%1 cette semaine").arg(currentUser.workoutSessions);
                    } else if (statName == "Calories brûlées") {
                        newValue = QString("%1 kcal").arg(currentUser.caloriesBurned);
                    } else if (statName == "Minutes d'activité") {
                        newValue = QString("%1 min").arg(currentUser.activityMinutes);
                    }

                    if (!newValue.isEmpty()) {
                        statValueLabel->setStyleSheet("font-weight: bold; color: #4cc9f0; padding: 4px;");
                        statValueLabel->setText(newValue);
                        QTimer::singleShot(500, [statValueLabel]() {
                            statValueLabel->setStyleSheet("font-weight: bold; color: #2b2d42; padding: 4px;");
                        });
                    }
                }
                break;
            }
        }
    }

    QWidget* goalsWidget = nullptr;
    for (int i = 0; i < rightSidebar->layout()->count(); ++i) {
        QWidget* widget = rightSidebar->layout()->itemAt(i)->widget();
        if (widget && widget->objectName() == "goalsWidget") {
            goalsWidget = widget;
            break;
        }
    }

    if (goalsWidget) {
        QLayout* goalList = goalsWidget->layout();
        for (int i = 1; i < goalList->count(); ++i) {
            QWidget* goalItem = goalList->itemAt(i)->widget();
            if (goalItem) {
                QLayout* goalLayout = goalItem->layout();
                if (goalLayout && goalLayout->count() >= 2) {
                    QHBoxLayout* labelLayout = qobject_cast<QHBoxLayout*>(goalLayout->itemAt(0)->layout());
                    QProgressBar* progressBar = qobject_cast<QProgressBar*>(goalLayout->itemAt(1)->widget());

                    if (labelLayout && progressBar && labelLayout->count() >= 3) {
                        QLabel* goalNameLabel = qobject_cast<QLabel*>(labelLayout->itemAt(0)->widget());
                        QLabel* goalValueLabel = qobject_cast<QLabel*>(labelLayout->itemAt(2)->widget());

                        if (goalNameLabel && goalValueLabel) {
                            QString goalName = goalNameLabel->text();
                            int progress = currentUser.goals.value(goalName, 0);

                            goalValueLabel->setStyleSheet("font-weight: bold; color: #4cc9f0;");
                            goalValueLabel->setText(QString("%1%").arg(progress));

                            QPropertyAnimation* animation = new QPropertyAnimation(progressBar, "value");
                            animation->setDuration(500);
                            animation->setStartValue(progressBar->value());
                            animation->setEndValue(progress);
                            animation->setEasingCurve(QEasingCurve::OutCubic);
                            animation->start(QAbstractAnimation::DeleteWhenStopped);

                            QTimer::singleShot(300, [goalValueLabel]() {
                                goalValueLabel->setStyleSheet("font-weight: bold; color: #2b2d42;");
                            });
                        }
                    }
                }
            }
        }
    }
}

void DashboardWindow::finishExercise() {
    if (isTimerRunning) {
        timer->stop(); // Ensure timer is stopped
        isTimerRunning = false;
    }

    updateUserStats();
    QMessageBox::information(this, "Exercice terminé",
                             QString("Exercice terminé avec succès!\n"
                                     "Temps d'activité: %1 minutes\n"
                                     "Calories brûlées: %2")
                                 .arg(exerciseDuration / 60)
                                 .arg(exerciseDuration / 60 * caloriesPerMinute));

    currentSet = 1;
    setCountLabel->setText(QString("Série %1/%2").arg(currentSet).arg(totalSets));
    remainingTime = 30;
    exerciseInProgress = false;
    exerciseDuration = 0;
    updateTimerDisplay();
    updateStatisticsDisplay();

    // Delay the showMaximized call to avoid flicker
    QTimer::singleShot(100, this, [this]() {
        showMaximized();
    });
}
void DashboardWindow::loadUserData()
{
    // Si l'ID n'est pas valide (par ex. mode déconnecté), on ne fait rien.
    if (currentUser.userId == -1) {
        return;
    }

    // Charger les statistiques et les objectifs depuis la base de données
    DatabaseManager::instance().loadUserStats(currentUser.userId, currentUser.workoutSessions,
                                              currentUser.caloriesBurned, currentUser.activityMinutes,
                                              currentUser.exercisesDone);

    DatabaseManager::instance().loadUserGoals(currentUser.userId, currentUser.goals);

    // S'assurer que les objectifs existent même si la DB est vide pour cet utilisateur
    if (currentUser.goals.isEmpty()) {
        currentUser.goals["Perte de poids"] = 0;
        currentUser.goals["Musculation"] = 0;
        // currentUser.goals["Cardio"] = 0;
    }
}
