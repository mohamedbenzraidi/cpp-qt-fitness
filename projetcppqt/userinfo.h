#ifndef USERINFO_H
#define USERINFO_H

#include <QString>
#include <QMap>

// Cette structure contiendra toutes les informations sur l'utilisateur connecté.
// Elle est créée dans LoginWindow et passée à DashboardWindow.
struct UserInfo {
    int userId = -1;
    QString name = "Utilisateur";
    QString planType = "Standard";

    // Statistiques
    int workoutSessions = 0;
    int caloriesBurned = 0;
    int activityMinutes = 0;
    int exercisesDone = 0;

    // Objectifs
    QMap<QString, int> goals;
};

#endif // USERINFO_H
