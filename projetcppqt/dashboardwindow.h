#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QProgressBar>
#include <QTimer>
#include <QMap>
#include <QList>
#include <QTime>
#include <QPropertyAnimation>
#include <QEasingCurve>

#include "mealplanview.h"
#include "habitsview.h"
#include "waterwidget.h"


// Structure to store user information
struct UserInfo {
    QString name;
    QString planType;
    int workoutSessions;
    int caloriesBurned;
    int activityMinutes;
    int exercisesDone;
    int userId;
    QMap<QString, int> goals;
};

// Structure to store exercise information
struct ExerciseInfo {
    QString name;
    QString description;
    QString imagePath;
    int caloriesPerMinute;
    int defaultDuration;
    int defaultSets;
    QString muscleGroupImagePath;
};

class DashboardWindow : public QMainWindow {
    Q_OBJECT

public:
    // Constructors
    DashboardWindow(QWidget *parent = nullptr);
    DashboardWindow(const UserInfo& userInfo, QWidget *parent = nullptr);
    ~DashboardWindow();
    void finishExercise();

private slots:
    // Navigation
    void switchToView(int viewIndex);

    // Exercise carousel methods
    void nextExercise();
    void previousExercise();
    void selectCurrentExercise();
    void updateExerciseDisplay();

    // Timer control
    void startTimer();
    void pauseTimer();
    void resetTimer();
    void updateTimer();

    // Update UI elements
    void updateStatisticsDisplay();
    void updateTimerDisplay();

private:
    void loadUserData();
    // UI setup methods
    void setupUI();
    void setupNavSidebar();
    void setupMainContent();
    void setupRightSidebar();
    void setupExerciseView();
    void setupExercisesList();

    // UI section creation methods
    QWidget* createStreakSection();
    QWidget* createTimerSection();
    QWidget* createMuscleSection();
    QWidget* createDataCardsSection();

    // Exercise methods
    void startExercise(const QString& exerciseName, int sets, int duration);
    void setExerciseType(const QString& exerciseName, int caloriesRate);

    // Stats tracking methods
    void updateUserStats();
    int calculateCaloriesBurned();
    void saveUserStatsToDatabase();
    void animateStatsUpdate();

    void saveUserStats();
    void animateStats();
    void displayExerciseWidgets();
    // Main UI components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QWidget *navSidebar;
    QStackedWidget *mainContent;
    QWidget *rightSidebar;

    // View components
    QWidget *exerciseView;
    MealPlanView *mealPlanView;
    HabitsView *habitsView;
    WaterWidget *waterView;

    // Exercise selection components
    QWidget *exerciseCarouselWidget;
    QWidget *exerciseCardWidget;
    QLabel *exerciseImageLabel;
    QLabel *exerciseNameLabel;
    QLabel *exerciseDescLabel;
    QPushButton *prevExerciseButton;
    QPushButton *nextExerciseButton;

    // Timer components
    QTimer *timer;
    QLabel *timerLabel;
    QLabel *timerDisplayValue;
    QLabel *setCountLabel;
    QLabel *activityLabel;

    // Exercise data
    QList<ExerciseInfo> exercisesList;
    int currentExerciseIndex = 0;

    // Timer state
    int remainingTime;
    bool isTimerRunning;
    int currentSet;
    int totalSets;

    // Exercise tracking
    int exerciseDuration;
    int caloriesPerMinute;
    bool exerciseInProgress;
    QTime exerciseStartTime;

    // User data
    UserInfo currentUser;

     QLabel *muscleMapLabel;
};

#endif // DASHBOARDWINDOW_H
