#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVariantMap>

#include <QMap>
#include <QSet>
#include <QDate>
#include "MealPlanView.h"
#include "HabitsView.h"
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();
    ~DatabaseManager();
    bool cleanupOrphanedData();
    bool verifyDataIntegrity(int userId);
    void debugMealData(int userId);
    bool saveWaterData(int userId, const QString &date, int dailyGoal, int currentAmount);
    bool loadWaterData(int userId, const QString &date, int &dailyGoal, int &currentAmount);
    bool saveMeal(int userId, int dayOfWeek, const QString &name, const QString &time, int calories, const QString &imagePath, const QList<QPair<QString, QString>> &ingredients);
    bool loadMeals(int userId, QMap<int, QList<MealPlanView::MealInfo>> &meals);
    bool saveExercise(int userId, int dayOfWeek, const QString &name, const QString &duration, int calories, bool completed);
    bool loadExercises(int userId, QMap<int, QList<MealPlanView::ExerciseInfo>> &exercises);

    bool updateUserStats(int userId, int workoutSessions, int caloriesBurned, int activityMinutes, int exercisesDone);
    bool loadUserStats(int userId, int &workoutSessions, int &caloriesBurned, int &activityMinutes, int &exercisesDone);
    bool saveUserGoals(int userId, const QMap<QString, int> &goals);
    bool loadUserGoals(int userId, QMap<QString, int> &goals);
    bool saveHabit(int userId, int habitId, const QString &name, int goalDays, const QSet<QDate> &completedDates);
    bool loadHabits(int userId, QMap<int, QMap<QString, QVariant>> &habits);
    bool deleteHabit(int userId, int habitId);
    int getNextHabitId(int userId);

    bool getUserInfo(const QString &email, QString &firstName, QString &lastName,
                     int &age, double &weight, double &height, QString &fitnessLevel);
    bool openDatabase();
    void closeDatabase();
    bool isOpen() const;

    bool createUser(const QString &firstName, const QString &lastName,
                    const QString &email, const QString &password,
                    int age, double weight, double height,
                    const QString &fitnessLevel);
    bool checkCredentials(const QString &email, const QString &password);
    int getUserId(const QString &email); // Récupère l'ID basé sur l'email

    // Nouvelles méthodes pour gérer l'utilisateur courant
    void setCurrentUserId(int userId); // Pour définir l'utilisateur après connexion
    int getCurrentUserId() const;      // Pour que DashboardWindow le récupère

    // Méthode pour récupérer toutes les données utilisateur utiles pour Dashboard
    // Retourne un QVariantMap pour la flexibilité, ou vous pourriez créer une struct UserData
    QVariantMap getUserData(int userId);
    explicit DatabaseManager(QObject *parent = nullptr);
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    QString getUserName(int userId);
    QString getUserPlanType(int userId);

private:


    bool createTables();

    QSqlDatabase m_database;
    QString m_databasePath;
    bool m_isInitialized;
    int m_currentLoggedInUserId; // Pour stocker l'ID de l'utilisateur connecté
};

#endif // DATABASEMANAGER_H
