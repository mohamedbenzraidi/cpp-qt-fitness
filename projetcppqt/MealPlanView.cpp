#include "mealplanview.h"
#include "databasemanager.h"
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

MealPlanView::MealPlanView(int userId, QWidget *parent) : QWidget(parent), m_userId(userId) {
    selectedDate = QDate::currentDate();
    qDebug() << "Initializing MealPlanView for user:" << userId;
    qDebug() << "Current date:" << selectedDate.toString();

    loadWeeklyMealsFromDatabase();
    debugMealData(); // Ajoutez cette ligne pour debug

    setupUI();
    updateMealsForCurrentDay();
}
MealPlanView::~MealPlanView() {
    // Nettoyage si nécessaire
}



void MealPlanView::setupUI() {
    // Création du conteneur principal avec scrolling
    scrollArea = new QScrollArea(this);
    contentWidget = new QWidget();
    mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Configuration du widget principal
    setObjectName("mealPlanView");
    setStyleSheet("QWidget#mealPlanView { background-color: #f8f9fa; }");

    // Configuration du scroll area
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Layout principal pour contenir le scroll area
    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(scrollArea);

    // Ajout des différentes sections
    setupHeader();
    setupNutritionOverview();
    setupMeals();
     //setupExerciseTracker();

    // Ajout d'un espace en fin de layout
    mainLayout->addStretch();
}

void MealPlanView::setupHeader() {
    // Widget conteneur pour l'en-tête
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    // Section titre
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLabel = new QLabel("Plan alimentaire");
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #2b2d42;");

    dateLabel = new QLabel();
    dateLabel->setStyleSheet("font-size: 16px; color: #8d99ae;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(dateLabel);

    // Boutons de navigation
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setSpacing(10);

    prevDayButton = new QPushButton("← Jour précédent");
    prevDayButton->setStyleSheet("background-color: #4361ee; color: white; border: none; border-radius: 8px; padding: 10px 20px; font-weight: bold;");
    prevDayButton->setCursor(Qt::PointingHandCursor);

    nextDayButton = new QPushButton("Jour suivant →");
    nextDayButton->setStyleSheet("background-color: #4cc9f0; color: white; border: none; border-radius: 8px; padding: 10px 20px; font-weight: bold;");
    nextDayButton->setCursor(Qt::PointingHandCursor);

    connect(prevDayButton, &QPushButton::clicked, this, &MealPlanView::onPreviousDay);
    connect(nextDayButton, &QPushButton::clicked, this, &MealPlanView::onNextDay);

    navLayout->addWidget(prevDayButton);
    navLayout->addWidget(nextDayButton);
    navLayout->addStretch();

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addLayout(navLayout);

    mainLayout->addWidget(headerWidget);

    // Ligne séparatrice
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("background-color: #e9ecef; max-height: 1px; margin: 15px 0;");
    mainLayout->addWidget(separator);
}

void MealPlanView::setupNutritionOverview() {
    // Widget pour contenir les informations nutritionnelles
    QWidget *nutritionWidget = new QWidget();
    nutritionWidget->setObjectName("nutritionOverview");
    nutritionWidget->setStyleSheet("QWidget#nutritionOverview { background-color: white; border-radius: 15px; padding: 20px; }");
    QVBoxLayout *nutritionLayout = new QVBoxLayout(nutritionWidget);

    // Titre de la section
    QLabel *nutritionTitle = new QLabel("Résumé nutritionnel");
    nutritionTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2b2d42;");
    nutritionLayout->addWidget(nutritionTitle);

    // Calories consommées vs objectif
    QHBoxLayout *caloriesLayout = new QHBoxLayout();
    caloriesOverviewLabel = new QLabel();
    caloriesOverviewLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #4cc9f0;");

    caloriesProgressBar = new QProgressBar();
    caloriesProgressBar->setTextVisible(false);
    caloriesProgressBar->setStyleSheet("QProgressBar { background-color: #e9ecef; border-radius: 7px; height: 14px; } "
                                       "QProgressBar::chunk { background-color: #4cc9f0; border-radius: 7px; }");

    caloriesLayout->addWidget(caloriesOverviewLabel);
    caloriesLayout->addWidget(caloriesProgressBar, 1);
    nutritionLayout->addLayout(caloriesLayout);

    // Macronutriments
    QLabel *macrosTitle = new QLabel("Macronutriments");
    macrosTitle->setStyleSheet("font-size: 16px; color: #8d99ae; margin-top: 15px;");
    nutritionLayout->addWidget(macrosTitle);

    QHBoxLayout *macrosLayout = new QHBoxLayout();
    macrosLayout->setSpacing(15);

    macrosLayout->addWidget(createMacroWidget("Protéines", 32, "125g", "#4cc9f0"), 1);
    macrosLayout->addWidget(createMacroWidget("Glucides", 43, "250g", "#4361ee"), 1);
    macrosLayout->addWidget(createMacroWidget("Lipides", 25, "70g", "#3a0ca3"), 1);

    nutritionLayout->addLayout(macrosLayout);
    mainLayout->addWidget(nutritionWidget);
}

QWidget* MealPlanView::createMacroWidget(const QString &name, int percentage, const QString &value, const QString &colorHex) {
    QWidget *macroWidget = new QWidget();
    QVBoxLayout *macroLayout = new QVBoxLayout(macroWidget);
    macroLayout->setSpacing(5);

    // Titre et pourcentage
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-size: 14px; color: #2b2d42;");

    QLabel *percentLabel = new QLabel(QString("%1%").arg(percentage));
    percentLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: " + colorHex + ";");
    percentLabel->setAlignment(Qt::AlignRight);

    headerLayout->addWidget(nameLabel);
    headerLayout->addWidget(percentLabel);
    macroLayout->addLayout(headerLayout);

    // Barre de progression
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(percentage);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(8);
    progressBar->setStyleSheet("QProgressBar { background-color: #e9ecef; border-radius: 4px; } "
                               "QProgressBar::chunk { background-color: " + colorHex + "; border-radius: 4px; }");
    macroLayout->addWidget(progressBar);

    // Valeur
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2b2d42;");
    macroLayout->addWidget(valueLabel);

    return macroWidget;
}

void MealPlanView::setupMeals() {
    // Titre de section
    QLabel *mealsTitle = new QLabel("Repas du jour");
    mealsTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2b2d42; margin-top: 10px;");
    mainLayout->addWidget(mealsTitle);
}

QWidget* MealPlanView::createMealCard(const MealInfo &meal) {
    QWidget *card = new QWidget();
    card->setObjectName("mealCard");
    card->setStyleSheet("QWidget#mealCard { background-color: white; border-radius: 15px; padding: 20px; }");
    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setSpacing(20);

    // Image du repas
    QLabel *imageLabel = new QLabel();
    QPixmap mealPixmap(meal.image);
    if (mealPixmap.isNull()) {
        // Affichage par défaut si l'image n'est pas trouvée
        imageLabel->setText("[Image]");
        imageLabel->setStyleSheet("font-size: 18px; color: #8d99ae; border: 2px dashed #d3d3d3; border-radius: 10px; padding: 30px;");
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setFixedSize(100, 100);
    } else {
        imageLabel->setPixmap(mealPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setFixedSize(100, 100);
    }

    cardLayout->addWidget(imageLabel);

    // Informations sur le repas
    QWidget *infoWidget = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setSpacing(8);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    // En-tête avec nom et heure
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(meal.name);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2b2d42;");

    QLabel *timeLabel = new QLabel(meal.time);
    timeLabel->setStyleSheet("font-size: 16px; color: #8d99ae;");
    timeLabel->setAlignment(Qt::AlignRight);

    headerLayout->addWidget(nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(timeLabel);
    infoLayout->addLayout(headerLayout);

    // Liste des ingrédients
    QGridLayout *ingredientsLayout = new QGridLayout();
    ingredientsLayout->setSpacing(8);

    for (int i = 0; i < meal.ingredients.size(); ++i) {
        QLabel *ingredientLabel = new QLabel(meal.ingredients[i].first);
        ingredientLabel->setStyleSheet("font-size: 14px; color: #2b2d42;");

        QLabel *quantityLabel = new QLabel(meal.ingredients[i].second);
        quantityLabel->setStyleSheet("font-size: 14px; color: #8d99ae;");
        quantityLabel->setAlignment(Qt::AlignRight);

        ingredientsLayout->addWidget(ingredientLabel, i, 0);
        ingredientsLayout->addWidget(quantityLabel, i, 1);
    }

    infoLayout->addLayout(ingredientsLayout);
    cardLayout->addWidget(infoWidget, 1);

    // Calories
    QWidget *caloriesWidget = new QWidget();
    QVBoxLayout *caloriesLayout = new QVBoxLayout(caloriesWidget);
    caloriesLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);

    QLabel *caloriesLabel = new QLabel(meal.calories);
    caloriesLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #4cc9f0;");

    QPushButton *detailsButton = new QPushButton("Détails");
    detailsButton->setStyleSheet("background-color: #f8f9fa; color: #4361ee; border: 1px solid #4361ee; border-radius: 5px; padding: 5px 15px;");
    detailsButton->setCursor(Qt::PointingHandCursor);

    caloriesLayout->addWidget(caloriesLabel);
    caloriesLayout->addWidget(detailsButton);
    caloriesLayout->addStretch();

    cardLayout->addWidget(caloriesWidget);

    return card;
}

// void MealPlanView::setupExerciseTracker() {
//     // Widget pour le suivi d'exercices
//     exerciseTrackWidget = new QWidget();
//     exerciseTrackWidget->setObjectName("exerciseTracker");
//     exerciseTrackWidget->setStyleSheet("QWidget#exerciseTracker { "
//                                        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
//                                        "stop:0 #ffffff, stop:1 #f8f9fa); "
//                                        "border-radius: 15px; "
//                                        "padding: 20px; "
//                                        "border: 1px solid #e9ecef; }");
//     QVBoxLayout *exerciseLayout = new QVBoxLayout(exerciseTrackWidget);

//     // Titre avec icône
//     QHBoxLayout *titleLayout = new QHBoxLayout();
//     QLabel *exerciseIcon = new QLabel("🏃‍♂️");
//     exerciseIcon->setStyleSheet("font-size: 24px;");

//     QLabel *exerciseTitle = new QLabel("Exercices du jour");
//     exerciseTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2b2d42; margin-left: 10px;");

//     titleLayout->addWidget(exerciseIcon);
//     titleLayout->addWidget(exerciseTitle);
//     titleLayout->addStretch();

//     exerciseProgressLabel = new QLabel();
//     exerciseProgressLabel->setStyleSheet("font-size: 16px; color: #8d99ae; margin-top: 5px;");

//     exerciseLayout->addLayout(titleLayout);
//     exerciseLayout->addWidget(exerciseProgressLabel);

//     mainLayout->addWidget(exerciseTrackWidget);
// }

// void MealPlanView::updateMealsForCurrentDay() {
//     // Mettre à jour la date affichée
//     dateLabel->setText(selectedDate.toString("dddd d MMMM yyyy"));

//     // Récupérer les repas pour le jour sélectionné
//     int dayOfWeek = selectedDate.dayOfWeek();
//     currentMeals = getMealsForDay(dayOfWeek);
//     currentExercises = getExercisesForDay(dayOfWeek);

//     // Supprimer les anciennes cartes de repas
//     QList<QWidget*> mealCards = findChildren<QWidget*>("mealCard");
//     for (QWidget* card : mealCards) {
//         card->deleteLater();
//     }

//     // Créer les nouvelles cartes de repas
//     for (const MealInfo &meal : currentMeals) {
//         QWidget *mealCard = createMealCard(meal);
//         mainLayout->insertWidget(mainLayout->count() - 2, mealCard); // Insérer avant exerciseTracker et stretch
//     }

//     // Mettre à jour les données nutritionnelles et d'exercices
//     updateNutritionData();
//     //updateExerciseData();
// }

QList<MealPlanView::MealInfo> MealPlanView::getMealsForDay(int dayOfWeek) {
    if (weeklyMeals.contains(dayOfWeek)) {
        return weeklyMeals[dayOfWeek];
    }
    return QList<MealInfo>(); // Retourner une liste vide si pas de données
}

QList<MealPlanView::ExerciseInfo> MealPlanView::getExercisesForDay(int dayOfWeek) {
    if (weeklyExercises.contains(dayOfWeek)) {
        return weeklyExercises[dayOfWeek];
    }
    return QList<ExerciseInfo>(); // Retourner une liste vide si pas de données
}

void MealPlanView::updateNutritionData() {
    // Calculer les calories totales pour le jour
    int totalCalories = 0;
    for (const MealInfo &meal : currentMeals) {
        QString caloriesStr = meal.calories;
        caloriesStr.remove(" kcal");
        totalCalories += caloriesStr.toInt();
    }

    int targetCalories = 2200;
    caloriesOverviewLabel->setText(QString("%1 / %2 kcal").arg(totalCalories).arg(targetCalories));

    caloriesProgressBar->setRange(0, targetCalories);
    caloriesProgressBar->setValue(totalCalories);
}

// void MealPlanView::updateExerciseData() {
//     // Supprimer les anciens boutons d'exercices
//     for (QPushButton* button : exerciseButtons) {
//         button->deleteLater();
//     }
//     exerciseButtons.clear();

//     // Calculer les statistiques d'exercices
//     int completedExercises = 0;
//     int totalCaloriesBurned = 0;

//     QVBoxLayout *exerciseLayout = qobject_cast<QVBoxLayout*>(exerciseTrackWidget->layout());

//     // Créer un widget conteneur pour tous les exercices
//     QWidget *exercisesContainer = new QWidget();
//     QVBoxLayout *containerLayout = new QVBoxLayout(exercisesContainer);
//     containerLayout->setSpacing(10);
//     containerLayout->setContentsMargins(0, 15, 0, 0);

//     // Ajouter les exercices du jour
//     for (int i = 0; i < currentExercises.size(); ++i) {
//         const ExerciseInfo &exercise = currentExercises[i];

//         // Widget pour chaque exercice avec style amélioré
//         QWidget *exerciseItemWidget = new QWidget();
//         exerciseItemWidget->setStyleSheet("QWidget { "
//                                           "background-color: #ffffff; "
//                                           "border-radius: 10px; "
//                                           "padding: 15px; "
//                                           "border: 1px solid #e9ecef; }");
//         QHBoxLayout *itemLayout = new QHBoxLayout(exerciseItemWidget);
//         itemLayout->setContentsMargins(10, 10, 10, 10);

//         // Icône d'exercice
//         QLabel *exerciseTypeIcon = new QLabel();
//         if (exercise.name.contains("Course") || exercise.name.contains("Marche")) {
//             exerciseTypeIcon->setText("🏃");
//         } else if (exercise.name.contains("Vélo")) {
//             exerciseTypeIcon->setText("🚴");
//         } else if (exercise.name.contains("Natation")) {
//             exerciseTypeIcon->setText("🏊");
//         } else if (exercise.name.contains("Yoga") || exercise.name.contains("Étirements")) {
//             exerciseTypeIcon->setText("🧘");
//         } else {
//             exerciseTypeIcon->setText("💪");
//         }
//         exerciseTypeIcon->setStyleSheet("font-size: 20px; margin-right: 10px;");

//         // Informations de l'exercice
//         QVBoxLayout *infoLayout = new QVBoxLayout();
//         QLabel *nameLabel = new QLabel(exercise.name);
//         nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2b2d42;");

//         QLabel *detailsLabel = new QLabel(QString("%1 - %2").arg(exercise.duration, exercise.calories));
//         detailsLabel->setStyleSheet("font-size: 14px; color: #8d99ae; margin-top: 2px;");

//         infoLayout->addWidget(nameLabel);
//         infoLayout->addWidget(detailsLabel);
//         infoLayout->setSpacing(3);

//         // Bouton de validation sans animation
//         QPushButton *completeButton = new QPushButton();
//         completeButton->setProperty("exerciseIndex", i);

//         if (exercise.completed) {
//             completeButton->setText("✅ Terminé");
//             completeButton->setStyleSheet("background-color: #28a745; "
//                                           "color: white; "
//                                           "border: none; "
//                                           "border-radius: 8px; "
//                                           "padding: 10px 20px; "
//                                           "font-weight: bold; "
//                                           "font-size: 14px;");
//             completeButton->setEnabled(false);
//             completedExercises++;

//             QString caloriesStr = exercise.calories;
//             caloriesStr.remove(" kcal");
//             totalCaloriesBurned += caloriesStr.toInt();
//         } else {
//             completeButton->setText("Marquer fait");
//             completeButton->setStyleSheet("background-color: #4cc9f0; "
//                                           "color: white; "
//                                           "border: none; "
//                                           "border-radius: 8px; "
//                                           "padding: 10px 20px; "
//                                           "font-weight: bold; "
//                                           "font-size: 14px; "
//                                           "transition: all 0.3s ease;");
//             completeButton->setCursor(Qt::PointingHandCursor);
//             connect(completeButton, &QPushButton::clicked, this, &MealPlanView::onExerciseCompleted);
//         }

//         exerciseButtons.append(completeButton);

//         itemLayout->addWidget(exerciseTypeIcon);
//         itemLayout->addLayout(infoLayout, 1);
//         itemLayout->addWidget(completeButton);

//         containerLayout->addWidget(exerciseItemWidget);
//     }

//     // Mettre à jour le label de progression avec plus de style
//     QString progressText = QString("🎯 Progression: %1/%2 exercices | 🔥 %3 kcal brûlées")
//                                .arg(completedExercises)
//                                .arg(currentExercises.size())
//                                .arg(totalCaloriesBurned);
//     exerciseProgressLabel->setText(progressText);

//     // Barre de progression stylée
//     QProgressBar *exerciseProgress = new QProgressBar();
//     exerciseProgress->setRange(0, currentExercises.size() == 0 ? 1 : currentExercises.size());
//     exerciseProgress->setValue(completedExercises);
//     exerciseProgress->setTextVisible(false);
//     exerciseProgress->setFixedHeight(8);
//     exerciseProgress->setStyleSheet("QProgressBar { "
//                                     "background-color: #e9ecef; "
//                                     "border-radius: 4px; "
//                                     "margin: 15px 0; } "
//                                     "QProgressBar::chunk { "
//                                     "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
//                                     "stop:0 #28a745, stop:1 #20c997); "
//                                     "border-radius: 4px; }");

//     // Ajouter tous les éléments au layout principal
//     if (exerciseLayout->count() > 2) {
//         // Supprimer les anciens widgets (sauf titre et sous-titre)
//         while (exerciseLayout->count() > 2) {
//             QLayoutItem *item = exerciseLayout->takeAt(2);
//             if (item->widget()) {
//                 item->widget()->deleteLater();
//             }
//             delete item;
//         }
//     }

//     exerciseLayout->addWidget(exerciseProgress);
//     exerciseLayout->addWidget(exercisesContainer);
// }

void MealPlanView::onPreviousDay() {
    selectedDate = selectedDate.addDays(-1);
    updateMealsForCurrentDay();
}

void MealPlanView::onNextDay() {
    selectedDate = selectedDate.addDays(1);
    updateMealsForCurrentDay();
}

// void MealPlanView::onExerciseCompleted() {
//     QPushButton *button = qobject_cast<QPushButton*>(sender());
//     if (!button) return;

//     bool ok;
//     int exerciseIndex = button->property("exerciseIndex").toInt(&ok);
//     if (!ok || exerciseIndex < 0 || exerciseIndex >= currentExercises.size()) return;

//     currentExercises[exerciseIndex].completed = true;

//     button->setText("✓ Terminé");
//     button->setStyleSheet("background-color: #28a745; color: white; border: none; border-radius: 5px; padding: 8px 15px; font-weight: bold;");
//     button->setEnabled(false);

//     updateExerciseData();

//     int dayOfWeek = selectedDate.dayOfWeek();
//     if (weeklyExercises.contains(dayOfWeek) && exerciseIndex < weeklyExercises[dayOfWeek].size()) {
//         weeklyExercises[dayOfWeek][exerciseIndex].completed = true;
//         DatabaseManager dbManager;
//         const ExerciseInfo &exercise = weeklyExercises[dayOfWeek][exerciseIndex];
//         QString caloriesStr = exercise.calories; // Crée une copie
//         caloriesStr.remove(" kcal");             // Modifie la copie
//         int calories = caloriesStr.toInt();      // Convertit la copie
//         dbManager.saveExercise(m_userId, dayOfWeek, exercise.name, exercise.duration, calories, true);
//     }
// }
void MealPlanView::saveWeeklyMealsToDatabase() {
    DatabaseManager dbManager;
    for (auto it = weeklyMeals.constBegin(); it != weeklyMeals.constEnd(); ++it) {
        int dayOfWeek = it.key();
        for (const MealInfo &meal : it.value()) {
            QString caloriesStr = meal.calories;  // 1. Déclarer ET initialiser la copie
            caloriesStr.remove(" kcal");          // 2. Modifier la copie
            int calories = caloriesStr.toInt();   // 3. Utiliser la copie

            dbManager.saveMeal(m_userId, dayOfWeek, meal.name, meal.time, calories, meal.image, meal.ingredients);
        }
    }
    for (auto it = weeklyExercises.constBegin(); it != weeklyExercises.constEnd(); ++it) {
        int dayOfWeek = it.key();
        for (const ExerciseInfo &exercise : it.value()) {
            QString caloriesStr = exercise.calories; // 1. Déclarer ET initialiser la copie
            caloriesStr.remove(" kcal");             // 2. Modifier la copie
            int calories = caloriesStr.toInt();      // 3. Utiliser la copie

            dbManager.saveExercise(m_userId, dayOfWeek, exercise.name, exercise.duration, calories, exercise.completed);
        }
    }
}
void MealPlanView::initializeDefaultMeals() {
    // LUNDI
    weeklyMeals[1] = {
        {"Petit-déjeuner", "07:30", "380 kcal", ":/images/breakfast_mon.png",
         {{"Muesli", "60g"}, {"Lait écrémé", "200ml"}, {"Fraises", "100g"}, {"Noix", "20g"}}},
        {"Déjeuner", "12:30", "520 kcal", ":/images/lunch_mon.png",
         {{"Salade verte", "150g"}, {"Blanc de poulet", "120g"}, {"Quinoa", "80g"}, {"Avocat", "50g"}}},
        {"Collation", "16:00", "180 kcal", ":/images/snack_mon.png",
         {{"Yaourt grec", "125g"}, {"Amandes", "15g"}, {"Miel", "10g"}}},
        {"Dîner", "19:30", "450 kcal", ":/images/dinner_mon.png",
         {{"Saumon grillé", "100g"}, {"Brocolis", "150g"}, {"Patate douce", "120g"}}}
    };

    // MARDI
    weeklyMeals[2] = {
        {"Petit-déjeuner", "07:30", "350 kcal", ":/images/breakfast_tue.png",
         {{"Pain complet", "2 tranches"}, {"Avocat", "1/2"}, {"Œuf", "1"}, {"Tomate", "50g"}}},
        {"Déjeuner", "12:30", "480 kcal", ":/images/lunch_tue.png",
         {{"Riz brun", "80g"}, {"Légumes sautés", "200g"}, {"Tofu", "100g"}}},
        {"Collation", "16:00", "150 kcal", ":/images/snack_tue.png",
         {{"Pomme", "1 moyenne"}, {"Beurre d'amande", "15g"}}},
        {"Dîner", "19:30", "420 kcal", ":/images/dinner_tue.png",
         {{"Escalope de dinde", "120g"}, {"Courgettes", "150g"}, {"Riz basmati", "60g"}}}
    };

    // MERCREDI
    weeklyMeals[3] = {
        {"Petit-déjeuner", "07:30", "400 kcal", ":/images/breakfast_wed.png",
         {{"Flocons d'avoine", "50g"}, {"Banane", "1"}, {"Lait d'amande", "200ml"}, {"Graines de chia", "10g"}}},
        {"Déjeuner", "12:30", "550 kcal", ":/images/lunch_wed.png",
         {{"Salade de lentilles", "150g"}, {"Feta", "50g"}, {"Concombre", "100g"}, {"Huile d'olive", "10ml"}}},
        {"Collation", "16:00", "200 kcal", ":/images/snack_wed.png",
         {{"Smoothie", "250ml"}, {"Épinards", "30g"}, {"Mangue", "80g"}}},
        {"Dîner", "19:30", "480 kcal", ":/images/dinner_wed.png",
         {{"Cabillaud", "120g"}, {"Haricots verts", "150g"}, {"Pommes de terre", "100g"}}}
    };

    // JEUDI
    weeklyMeals[4] = {
        {"Petit-déjeuner", "07:30", "370 kcal", ":/images/breakfast_thu.png",
         {{"Pain aux céréales", "2 tranches"}, {"Fromage blanc", "100g"}, {"Miel", "15g"}, {"Noix", "10g"}}},
        {"Déjeuner", "12:30", "500 kcal", ":/images/lunch_thu.png",
         {{"Pâtes complètes", "80g"}, {"Sauce tomate", "100ml"}, {"Basilic frais", "5g"}, {"Parmesan", "20g"}}},
        {"Collation", "16:00", "160 kcal", ":/images/snack_thu.png",
         {{"Carotte", "100g"}, {"Hummus", "30g"}}},
        {"Dîner", "19:30", "440 kcal", ":/images/dinner_thu.png",
         {{"Bœuf maigre", "100g"}, {"Épinards", "150g"}, {"Quinoa", "60g"}}}
    };

    // VENDREDI
    weeklyMeals[5] = {
        {"Petit-déjeuner", "07:30", "390 kcal", ":/images/breakfast_fri.png",
         {{"Smoothie bowl", "300ml"}, {"Granola", "30g"}, {"Fruits rouges", "80g"}}},
        {"Déjeuner", "12:30", "530 kcal", ":/images/lunch_fri.png",
         {{"Sushi bowl", "250g"}, {"Saumon cru", "80g"}, {"Avocat", "50g"}, {"Concombre", "50g"}}},
        {"Collation", "16:00", "170 kcal", ":/images/snack_fri.png",
         {{"Yaourt", "125g"}, {"Granola", "20g"}}},
        {"Dîner", "19:30", "460 kcal", ":/images/dinner_fri.png",
         {{"Crevettes", "120g"}, {"Légumes grillés", "200g"}, {"Riz sauvage", "60g"}}}
    };

    // SAMEDI
    weeklyMeals[6] = {
        {"Petit-déjeuner", "08:00", "420 kcal", ":/images/breakfast_sat.png",
         {{"Pancakes", "2 pièces"}, {"Sirop d'érable", "15ml"}, {"Fruits frais", "100g"}}},
        {"Déjeuner", "13:00", "580 kcal", ":/images/lunch_sat.png",
         {{"Burger végétarien", "1"}, {"Frites de patate douce", "100g"}, {"Salade", "80g"}}},
        {"Collation", "16:30", "190 kcal", ":/images/snack_sat.png",
         {{"Trail mix", "40g"}}},
        {"Dîner", "20:00", "490 kcal", ":/images/dinner_sat.png",
         {{"Pizza maison", "2 parts"}, {"Salade verte", "100g"}}}
    };

    // DIMANCHE
    weeklyMeals[7] = {
        {"Petit-déjeuner", "08:30", "360 kcal", ":/images/breakfast_sun.png",
         {{"Œufs brouillés", "2"}, {"Toast complet", "1"}, {"Épinards", "50g"}}},
        {"Déjeuner", "13:30", "520 kcal", ":/images/lunch_sun.png",
         {{"Rôti de porc", "100g"}, {"Légumes rôtis", "200g"}, {"Purée", "80g"}}},
        {"Collation", "16:00", "140 kcal", ":/images/snack_sun.png",
         {{"Thé", "250ml"}, {"Biscuits avoine", "2"}}},
        {"Dîner", "19:00", "400 kcal", ":/images/dinner_sun.png",
         {{"Soupe de légumes", "300ml"}, {"Pain complet", "1 tranche"}, {"Fromage", "30g"}}}
    };
}

void MealPlanView::initializeDefaultExercises() {
    // LUNDI
    weeklyExercises[1] = {
        {"Course à pied", "30 min", "300 kcal", false},
        {"Pompes", "15 min", "150 kcal", false},
        {"Étirements", "10 min", "50 kcal", false}
    };

    // MARDI
    weeklyExercises[2] = {
        {"Vélo", "45 min", "400 kcal", false},
        {"Abdominaux", "20 min", "180 kcal", false},
        {"Yoga", "15 min", "80 kcal", false}
    };

    // MERCREDI
    weeklyExercises[3] = {
        {"Natation", "40 min", "450 kcal", false},
        {"Squats", "15 min", "160 kcal", false},
        {"Méditation", "10 min", "30 kcal", false}
    };

    // JEUDI
    weeklyExercises[4] = {
        {"Marche rapide", "60 min", "350 kcal", false},
        {"Développé couché", "25 min", "200 kcal", false},
        {"Relaxation", "15 min", "40 kcal", false}
    };

    // VENDREDI
    weeklyExercises[5] = {
        {"Danse", "45 min", "380 kcal", false},
        {"Burpees", "20 min", "220 kcal", false},
        {"Étirements", "10 min", "50 kcal", false}
    };

    // SAMEDI
    weeklyExercises[6] = {
        {"Randonnée", "90 min", "500 kcal", false},
        {"Pilates", "30 min", "180 kcal", false}
    };

    // DIMANCHE
    weeklyExercises[7] = {
        {"Repos actif", "30 min", "100 kcal", false},
        {"Yoga doux", "20 min", "80 kcal", false}
    };
}


void MealPlanView::loadWeeklyMealsFromDatabase() {
    DatabaseManager dbManager;

    // Videz d'abord les données existantes
    weeklyMeals.clear();
    weeklyExercises.clear();

    if (!dbManager.loadMeals(m_userId, weeklyMeals)) {
        qDebug() << "Failed to load meals from database, initializing defaults.";
        initializeDefaultMeals();
        saveWeeklyMealsToDatabase();
    } else {
        qDebug() << "Successfully loaded meals from database";
    }

    if (!dbManager.loadExercises(m_userId, weeklyExercises)) {
        qDebug() << "Failed to load exercises from database, initializing defaults.";
        initializeDefaultExercises();
        saveWeeklyMealsToDatabase();
    } else {
        qDebug() << "Successfully loaded exercises from database";
    }

    // Vérification finale
    if (weeklyMeals.isEmpty()) {
        qDebug() << "WARNING: weeklyMeals is still empty after loading/initialization!";
        initializeDefaultMeals(); // Force l'initialisation
    }
}
// Ajoutez ces méthodes de debug dans votre classe MealPlanView

void MealPlanView::updateMealsForCurrentDay() {
    // Mettre à jour la date affichée
    dateLabel->setText(selectedDate.toString("dddd d MMMM yyyy"));

    // Récupérer les repas pour le jour sélectionné
    int dayOfWeek = selectedDate.dayOfWeek();
    currentMeals = getMealsForDay(dayOfWeek);
    currentExercises = getExercisesForDay(dayOfWeek);

    // DEBUG: Vérifiez le contenu
    qDebug() << "Day of week:" << dayOfWeek;
    qDebug() << "Number of meals for today:" << currentMeals.size();
    qDebug() << "Total days in weeklyMeals:" << weeklyMeals.keys();

    for (int i = 0; i < currentMeals.size(); ++i) {
        qDebug() << "Meal" << i << ":" << currentMeals[i].name;
    }

    // Supprimer les anciennes cartes de repas
    QList<QWidget*> mealCards = findChildren<QWidget*>("mealCard");
    qDebug() << "Removing" << mealCards.size() << "old meal cards";
    for (QWidget* card : mealCards) {
        card->deleteLater();
    }

    // Créer les nouvelles cartes de repas
    for (const MealInfo &meal : currentMeals) {
        QWidget *mealCard = createMealCard(meal);
        mainLayout->insertWidget(mainLayout->count() - 1, mealCard); // Changé: - 1 au lieu de - 2
        qDebug() << "Added meal card for:" << meal.name;
    }

    // Mettre à jour les données nutritionnelles et d'exercices
    updateNutritionData();

    // Force un refresh de l'interface
    contentWidget->update();
    scrollArea->update();
}

// Méthode de debug pour vérifier les données
void MealPlanView::debugMealData() {
    qDebug() << "=== DEBUG MEAL DATA ===";
    qDebug() << "User ID:" << m_userId;
    qDebug() << "Selected date:" << selectedDate.toString();
    qDebug() << "Day of week:" << selectedDate.dayOfWeek();

    for (auto it = weeklyMeals.constBegin(); it != weeklyMeals.constEnd(); ++it) {
        qDebug() << "Day" << it.key() << "has" << it.value().size() << "meals";
        for (const MealInfo &meal : it.value()) {
            qDebug() << "  -" << meal.name << "at" << meal.time;
        }
    }
    qDebug() << "=====================";
}
void MealPlanView::forceRefresh() {
    loadWeeklyMealsFromDatabase();
    updateMealsForCurrentDay();
}
