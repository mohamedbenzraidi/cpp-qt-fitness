#ifndef MEALPLANVIEW_H
#define MEALPLANVIEW_H

#include <QtWidgets>
#include <QtCore>
#include <QtGui>
class DatabaseManager;

class MealPlanView : public QWidget {
    Q_OBJECT

public:
    explicit MealPlanView(int userId, QWidget *parent = nullptr);
    ~MealPlanView();
    // Structure pour les informations des repas
    struct MealInfo {
        QString name;
        QString time;
        QString calories;
        QString image;
        QList<QPair<QString, QString>> ingredients;
    };

    // Structure pour les exercices
    struct ExerciseInfo {
        QString name;
        QString duration;
        QString calories;
        bool completed;
    };


private slots:
    void onPreviousDay();
    void onNextDay();
    void onExerciseCompleted();

private:

    void initializeDefaultMeals();    // AJOUTER CETTE LIGNE
    void initializeDefaultExercises(); // AJOUTER CETTE LIGNE
    // Widgets principaux
    int m_userId;
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *mainLayout;

    // Widgets pour l'en-tête
    QLabel *titleLabel;
    QLabel *dateLabel;
    QPushButton *prevDayButton;
    QPushButton *nextDayButton;

    // Widgets pour les données nutritionnelles
    QLabel *caloriesOverviewLabel;
    QProgressBar *caloriesProgressBar;
    QWidget *macrosWidget;

    // Widget pour le suivi d'exercices
    QWidget *exerciseTrackWidget;
    QList<QPushButton*> exerciseButtons;
    QLabel *exerciseProgressLabel;

    // Date actuelle sélectionnée
    QDate selectedDate;

    // Méthodes d'initialisation
    void setupUI();
    void setupHeader();
    void setupNutritionOverview();
    void setupMeals();
     void setupExerciseTracker();
    void loadWeeklyMealsFromDatabase();
    void saveWeeklyMealsToDatabase();


    void updateMealsForCurrentDay();
    void debugMealData();
    void forceRefresh();
    // Méthodes pour créer les widgets
    QWidget* createMealCard(const MealInfo &meal);
    QWidget* createMacroWidget(const QString &name, int percentage, const QString &value, const QString &colorHex);

    // Méthodes pour gérer les données


    void updateNutritionData();
    void updateExerciseProgress();
     void updateExerciseData();
    QList<MealInfo> getMealsForDay(int dayOfWeek);
    QList<ExerciseInfo> getExercisesForDay(int dayOfWeek);

    // Données des repas pour chaque jour de la semaine
    QMap<int, QList<MealInfo>> weeklyMeals; // Key: day of week (1=Monday, 7=Sunday)
    QMap<int, QList<ExerciseInfo>> weeklyExercises;

    // Repas actuels affichés
    QList<MealInfo> currentMeals;
    QList<ExerciseInfo> currentExercises;
};

#endif // MEALPLANVIEW_H
