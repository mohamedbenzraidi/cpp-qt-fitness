#include "databasemanager.h"
#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent), m_isInitialized(false)
{
    // Créer le dossier de données s'il n'existe pas
    QDir dataDir(QDir::homePath() + "/.efitness");
    if (!dataDir.exists()) {
        dataDir.mkpath(".");
    }

    m_databasePath = dataDir.absolutePath() + "/fitness_data.db";

    // Configurer la connexion à la base de données
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(m_databasePath);
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase()
{
    // Ouvrir la connexion à la base de données
    if (!m_database.isOpen()) {
        if (!m_database.open()) {
            qDebug() << "Erreur lors de l'ouverture de la base de données:" << m_database.lastError().text();
            return false;
        }
    }

    // Créer les tables si elles n'existent pas
    if (!m_isInitialized) {
        if (createTables()) {
            m_isInitialized = true;
        } else {
            qDebug() << "Erreur lors de la création des tables";
            return false;
        }
    }

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::isOpen() const
{
    return m_database.isOpen();
}


bool DatabaseManager::createUser(const QString &firstName, const QString &lastName,
                                 const QString &email, const QString &password,
                                 int age, double weight, double height,
                                 const QString &fitnessLevel)
{
    if (!isOpen() && !openDatabase()) {
        qDebug() << "Erreur: Impossible d'ouvrir la base de données";
        return false;
    }

    // Validation des données d'entrée
    if (firstName.trimmed().isEmpty() || lastName.trimmed().isEmpty() || email.trimmed().isEmpty()) {
        qDebug() << "Erreur: Champs obligatoires manquants";
        return false;
    }

    if (password.length() < 6) {
        qDebug() << "Erreur: Mot de passe trop court";
        return false;
    }

    // Vérifier si l'email existe déjà
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
    checkQuery.addBindValue(email.trimmed());

    if (!checkQuery.exec()) {
        qDebug() << "Erreur lors de la vérification de l'email:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "Erreur: Email déjà utilisé:" << email;
        return false;
    }

    // Préparer la requête d'insertion
    QSqlQuery query;
    query.prepare("INSERT INTO users (first_name, last_name, email, password, age, weight, height, fitness_level) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    // Hasher le mot de passe
    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    // Lier les valeurs
    query.addBindValue(firstName.trimmed());
    query.addBindValue(lastName.trimmed());
    query.addBindValue(email.trimmed());
    query.addBindValue(hashedPassword);
    query.addBindValue(age);
    query.addBindValue(weight);
    query.addBindValue(height);
    query.addBindValue(fitnessLevel);

    // Debug des valeurs
    qDebug() << "Creating user with values:";
    qDebug() << "First Name:" << firstName.trimmed();
    qDebug() << "Last Name:" << lastName.trimmed();
    qDebug() << "Email:" << email.trimmed();
    qDebug() << "Age:" << age;
    qDebug() << "Weight:" << weight;
    qDebug() << "Height:" << height;
    qDebug() << "Fitness Level:" << fitnessLevel;

    // Exécuter la requête
    if (!query.exec()) {
        qDebug() << "Erreur lors de la création de l'utilisateur:" << query.lastError().text();
        qDebug() << "Query SQL:" << query.lastQuery();
        qDebug() << "Database error:" << query.lastError().databaseText();
        qDebug() << "Driver error:" << query.lastError().driverText();
        return false;
    }

    // Récupérer l'ID du nouvel utilisateur
    int userId = getUserId(email.trimmed());
    if (userId == -1) {
        qDebug() << "Erreur: Impossible de récupérer l'ID utilisateur après création";
        return false;
    }

    // Initialiser les objectifs par défaut
    QMap<QString, int> defaultGoals;
    defaultGoals["Perte de poids"] = 0;
    defaultGoals["Musculation"] = 0;
    defaultGoals["Cardio"] = 0;

    if (!saveUserGoals(userId, defaultGoals)) {
        qDebug() << "Erreur: Impossible de sauvegarder les objectifs par défaut";
        // Ne pas retourner false ici car l'utilisateur a été créé avec succès
        // Les objectifs peuvent être créés plus tard
    }

    qDebug() << "Utilisateur créé avec succès, ID:" << userId;
    return true;
}

bool DatabaseManager::checkCredentials(const QString &email, const QString &password)
{
    // Vérifier si la connexion à la base de données est ouverte
    if (!isOpen()) {
        if (!openDatabase()) {
            return false;
        }
    }

    QSqlQuery query;

    // Hasher le mot de passe pour la vérification
    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    // Préparer la requête de vérification
    query.prepare("SELECT id FROM users WHERE email = ? AND password = ?");
    query.addBindValue(email);
    query.addBindValue(hashedPassword);

    // Exécuter la requête
    if (query.exec() && query.next()) {
        return true; // Utilisateur trouvé
    }

    return false;
}

int DatabaseManager::getUserId(const QString &email)
{
    // Vérifier si la connexion à la base de données est ouverte
    if (!isOpen()) {
        if (!openDatabase()) {
            return -1;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE email = ?");
    query.addBindValue(email);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1; // Utilisateur non trouvé
}
bool DatabaseManager::getUserInfo(const QString &email, QString &firstName, QString &lastName,
                                  int &age, double &weight, double &height, QString &fitnessLevel)
{
    // Vérifier si la connexion à la base de données est ouverte
    if (!isOpen()) {
        if (!openDatabase()) {
            return false;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT first_name, last_name, age, weight, height, fitness_level FROM users WHERE email = ?");
    query.addBindValue(email);

    if (query.exec() && query.next()) {
        firstName = query.value(0).toString();
        lastName = query.value(1).toString();
        age = query.value(2).toInt();
        weight = query.value(3).toDouble();
        height = query.value(4).toDouble();
        fitnessLevel = query.value(5).toString();
        return true;
    }

    return false; // Utilisateur non trouvé
}

QString DatabaseManager::getUserName(int userId) {
    QSqlQuery query;
    query.prepare("SELECT first_name, last_name FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        QString firstName = query.value(0).toString();
        QString lastName = query.value(1).toString();
        return firstName + " " + lastName;
    }

    return "Utilisateur"; // Valeur par défaut si non trouvé
}

QString DatabaseManager::getUserPlanType(int userId) {
    QSqlQuery query;
    query.prepare("SELECT plan_type FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return "Standard"; // Valeur par défaut si non trouvé
}
bool DatabaseManager::createTables()
{
    QSqlQuery query;

    // Table utilisateurs (updated to include stats fields)
    bool success = query.exec("CREATE TABLE IF NOT EXISTS users ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "first_name TEXT NOT NULL, "
                              "last_name TEXT NOT NULL, "
                              "email TEXT UNIQUE NOT NULL, "
                              "password TEXT NOT NULL, "
                              "age INTEGER, "
                              "weight REAL, "
                              "height REAL, "
                              "fitness_level TEXT, "
                              "registration_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
                              "workout_sessions INTEGER DEFAULT 0, "
                              "calories_burned INTEGER DEFAULT 0, "
                              "activity_minutes INTEGER DEFAULT 0, "
                              "exercises_done INTEGER DEFAULT 0, "
                              "plan_type TEXT DEFAULT 'Standard'"
                              ")");

    if (!success) {
        qDebug() << "Erreur lors de la création de la table users:" << query.lastError().text();
        return false;
    }
    // Table water_intake
    success = query.exec("CREATE TABLE IF NOT EXISTS water_intake ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "user_id INTEGER NOT NULL, "
                         "date TEXT NOT NULL, "
                         "daily_goal INTEGER NOT NULL, "
                         "current_amount INTEGER NOT NULL, "
                         "FOREIGN KEY(user_id) REFERENCES users(id), "
                         "UNIQUE(user_id, date)"
                         ")");
    if (!success) {
        qDebug() << "Erreur lors de la création de la table water_intake:" << query.lastError().text();
        return false;
    }
    // Table user_goals
    success = query.exec("CREATE TABLE IF NOT EXISTS user_goals ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "user_id INTEGER NOT NULL, "
                         "goal_name TEXT NOT NULL, "
                         "progress INTEGER DEFAULT 0, "
                         "FOREIGN KEY(user_id) REFERENCES users(id), "
                         "UNIQUE(user_id, goal_name)"
                         ")");

    if (!success) {
        qDebug() << "Erreur lors de la création de la table user_goals:" << query.lastError().text();
        return false;
    }

    // Table habits
    success = query.exec("CREATE TABLE IF NOT EXISTS habits ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "user_id INTEGER NOT NULL, "
                         "habit_id INTEGER NOT NULL, "
                         "name TEXT NOT NULL, "
                         "goal_days INTEGER NOT NULL, "
                         "FOREIGN KEY(user_id) REFERENCES users(id), "
                         "UNIQUE(user_id, habit_id)"
                         ")");

    if (!success) {
        qDebug() << "Erreur lors de la création de la table habits:" << query.lastError().text();
        return false;
    }

    // Table meals
    success = query.exec("CREATE TABLE IF NOT EXISTS meals ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "user_id INTEGER NOT NULL, "
                         "day_of_week INTEGER NOT NULL, "
                         "name TEXT NOT NULL, "
                         "time TEXT NOT NULL, "
                         "calories INTEGER NOT NULL, "
                         "image_path TEXT, "
                         "FOREIGN KEY(user_id) REFERENCES users(id), "
                         "UNIQUE(user_id, day_of_week, name, time)"
                         ")");
    if (!success) {
        qDebug() << "Erreur lors de la création de la table meals:" << query.lastError().text();
        return false;
    }

    // Table meal_ingredients
    success = query.exec("CREATE TABLE IF NOT EXISTS meal_ingredients ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "meal_id INTEGER NOT NULL, "
                         "ingredient_name TEXT NOT NULL, "
                         "quantity TEXT NOT NULL, "
                         "FOREIGN KEY(meal_id) REFERENCES meals(id)"
                         ")");
    if (!success) {
        qDebug() << "Erreur lors de la création de la table meal_ingredients:" << query.lastError().text();
        return false;
    }

    // Table exercises
    success = query.exec("CREATE TABLE IF NOT EXISTS exercises ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "user_id INTEGER NOT NULL, "
                         "day_of_week INTEGER NOT NULL, "
                         "name TEXT NOT NULL, "
                         "duration TEXT NOT NULL, "
                         "calories INTEGER NOT NULL, "
                         "completed INTEGER NOT NULL DEFAULT 0, "
                         "FOREIGN KEY(user_id) REFERENCES users(id), "
                         "UNIQUE(user_id, day_of_week, name)"
                         ")");
    if (!success) {
        qDebug() << "Erreur lors de la création de la table exercises:" << query.lastError().text();
        return false;
    }
    // Table habit_completions
    success = query.exec("CREATE TABLE IF NOT EXISTS habit_completions ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "user_id INTEGER NOT NULL, "
                         "habit_id INTEGER NOT NULL, "
                         "completion_date DATE NOT NULL, "
                         "FOREIGN KEY(user_id) REFERENCES users(id), "
                         "UNIQUE(user_id, habit_id, completion_date)"
                         ")");

    if (!success) {
        qDebug() << "Erreur lors de la création de la table habit_completions:" << query.lastError().text();
        return false;
    }

    return true;
}
bool DatabaseManager::updateUserStats(int userId, int workoutSessions, int caloriesBurned, int activityMinutes, int exercisesDone)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE users SET workout_sessions = :sessions, "
                  "calories_burned = :calories, activity_minutes = :minutes, "
                  "exercises_done = :exercises WHERE id = :id");
    query.bindValue(":sessions", workoutSessions);
    query.bindValue(":calories", caloriesBurned);
    query.bindValue(":minutes", activityMinutes);
    query.bindValue(":exercises", exercisesDone);
    query.bindValue(":id", userId);

    if (!query.exec()) {
        qDebug() << "Error updating user stats:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::loadUserStats(int userId, int &workoutSessions, int &caloriesBurned, int &activityMinutes, int &exercisesDone)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT workout_sessions, calories_burned, activity_minutes, exercises_done "
                  "FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        workoutSessions = query.value(0).toInt();
        caloriesBurned = query.value(1).toInt();
        activityMinutes = query.value(2).toInt();
        exercisesDone = query.value(3).toInt();
        return true;
    }
    return false;
}

bool DatabaseManager::saveUserGoals(int userId, const QMap<QString, int> &goals)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    for (auto it = goals.constBegin(); it != goals.constEnd(); ++it) {
        query.prepare("INSERT OR REPLACE INTO user_goals (user_id, goal_name, progress) "
                      "VALUES (:user_id, :goal_name, :progress)");
        query.bindValue(":user_id", userId);
        query.bindValue(":goal_name", it.key());
        query.bindValue(":progress", it.value());

        if (!query.exec()) {
            qDebug() << "Error saving user goal:" << query.lastError().text();
            return false;
        }
    }
    return true;
}

bool DatabaseManager::loadUserGoals(int userId, QMap<QString, int> &goals)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT goal_name, progress FROM user_goals WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    goals.clear();
    if (query.exec()) {
        while (query.next()) {
            goals[query.value(0).toString()] = query.value(1).toInt();
        }
        return true;
    }
    qDebug() << "Error loading user goals:" << query.lastError().text();
    return false;
}

bool DatabaseManager::saveHabit(int userId, int habitId, const QString &name, int goalDays, const QSet<QDate> &completedDates)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;

    // Save or update habit
    query.prepare("INSERT OR REPLACE INTO habits (user_id, habit_id, name, goal_days) "
                  "VALUES (:user_id, :habit_id, :name, :goal_days)");
    query.bindValue(":user_id", userId);
    query.bindValue(":habit_id", habitId);
    query.bindValue(":name", name);
    query.bindValue(":goal_days", goalDays);

    if (!query.exec()) {
        qDebug() << "Error saving habit:" << query.lastError().text();
        return false;
    }

    // Clear existing completions for this habit
    query.prepare("DELETE FROM habit_completions WHERE user_id = :user_id AND habit_id = :habit_id");
    query.bindValue(":user_id", userId);
    query.bindValue(":habit_id", habitId);
    if (!query.exec()) {
        qDebug() << "Error clearing habit completions:" << query.lastError().text();
        return false;
    }

    // Save new completions
    for (const QDate &date : completedDates) {
        query.prepare("INSERT INTO habit_completions (user_id, habit_id, completion_date) "
                      "VALUES (:user_id, :habit_id, :date)");
        query.bindValue(":user_id", userId);
        query.bindValue(":habit_id", habitId);
        query.bindValue(":date", date.toString("yyyy-MM-dd"));
        if (!query.exec()) {
            qDebug() << "Error saving habit completion:" << query.lastError().text();
            return false;
        }
    }
    return true;
}

bool DatabaseManager::loadHabits(int userId, QMap<int, QMap<QString, QVariant>> &habits)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    habits.clear();

    // Load habits
    QSqlQuery habitQuery;
    habitQuery.prepare("SELECT habit_id, name, goal_days FROM habits WHERE user_id = :user_id");
    habitQuery.bindValue(":user_id", userId);

    if (habitQuery.exec()) {
        while (habitQuery.next()) {
            int habitId = habitQuery.value(0).toInt();
            QMap<QString, QVariant> habitData;
            habitData["name"] = habitQuery.value(1).toString();
            habitData["goalDays"] = habitQuery.value(2).toInt();

            // Load completions for this habit
            QSet<QDate> completedDates;
            QSqlQuery completionQuery;
            completionQuery.prepare("SELECT completion_date FROM habit_completions "
                                    "WHERE user_id = :user_id AND habit_id = :habit_id");
            completionQuery.bindValue(":user_id", userId);
            completionQuery.bindValue(":habit_id", habitId);

            if (completionQuery.exec()) {
                while (completionQuery.next()) {
                    QDate date = QDate::fromString(completionQuery.value(0).toString(), "yyyy-MM-dd");
                    if (date.isValid()) {
                        completedDates.insert(date);
                    }
                }
            } else {
                qDebug() << "Error loading habit completions:" << completionQuery.lastError().text();
                return false;
            }

            habitData["completedDates"] = QVariant::fromValue(completedDates);
            habits[habitId] = habitData;
        }
        return true;
    }
    qDebug() << "Error loading habits:" << habitQuery.lastError().text();
    return false;
}

bool DatabaseManager::deleteHabit(int userId, int habitId)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;

    // Delete habit completions
    query.prepare("DELETE FROM habit_completions WHERE user_id = :user_id AND habit_id = :habit_id");
    query.bindValue(":user_id", userId);
    query.bindValue(":habit_id", habitId);
    if (!query.exec()) {
        qDebug() << "Error deleting habit completions:" << query.lastError().text();
        return false;
    }

    // Delete habit
    query.prepare("DELETE FROM habits WHERE user_id = :user_id AND habit_id = :habit_id");
    query.bindValue(":user_id", userId);
    query.bindValue(":habit_id", habitId);
    if (!query.exec()) {
        qDebug() << "Error deleting habit:" << query.lastError().text();
        return false;
    }
    return true;
}

int DatabaseManager::getNextHabitId(int userId)
{
    if (!isOpen() && !openDatabase()) {
        return 1;
    }

    QSqlQuery query;
    query.prepare("SELECT MAX(habit_id) + 1 FROM habits WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (query.exec() && query.next()) {
        QVariant value = query.value(0);
        return value.isNull() ? 1 : value.toInt();
    }
    return 1;
}
// Corrections pour saveMeal() - Gestion propre des mises à jour
bool DatabaseManager::saveMeal(int userId, int dayOfWeek, const QString &name, const QString &time,
                               int calories, const QString &imagePath,
                               const QList<QPair<QString, QString>> &ingredients)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    int mealId = -1;

    // Vérifier si le meal existe déjà
    query.prepare("SELECT id FROM meals WHERE user_id = :user_id AND day_of_week = :day AND name = :name AND time = :time");
    query.bindValue(":user_id", userId);
    query.bindValue(":day", dayOfWeek);
    query.bindValue(":name", name);
    query.bindValue(":time", time);

    bool mealExists = false;
    if (query.exec() && query.next()) {
        mealId = query.value(0).toInt();
        mealExists = true;
    }

    if (mealExists) {
        // UPDATE du meal existant
        query.prepare("UPDATE meals SET calories = :calories, image_path = :image WHERE id = :id");
        query.bindValue(":calories", calories);
        query.bindValue(":image", imagePath);
        query.bindValue(":id", mealId);

        if (!query.exec()) {
            qDebug() << "Error updating meal:" << query.lastError().text();
            return false;
        }
    } else {
        // INSERT nouveau meal
        query.prepare("INSERT INTO meals (user_id, day_of_week, name, time, calories, image_path) "
                      "VALUES (:user_id, :day, :name, :time, :calories, :image)");
        query.bindValue(":user_id", userId);
        query.bindValue(":day", dayOfWeek);
        query.bindValue(":name", name);
        query.bindValue(":time", time);
        query.bindValue(":calories", calories);
        query.bindValue(":image", imagePath);

        if (!query.exec()) {
            qDebug() << "Error inserting meal:" << query.lastError().text();
            return false;
        }
        mealId = query.lastInsertId().toInt();
    }

    // Nettoyer les anciens ingrédients (même pour les nouveaux meals par sécurité)
    query.prepare("DELETE FROM meal_ingredients WHERE meal_id = :meal_id");
    query.bindValue(":meal_id", mealId);
    if (!query.exec()) {
        qDebug() << "Error clearing meal ingredients:" << query.lastError().text();
        return false;
    }

    // Sauvegarder les nouveaux ingrédients
    for (const auto &ingredient : ingredients) {
        query.prepare("INSERT INTO meal_ingredients (meal_id, ingredient_name, quantity) "
                      "VALUES (:meal_id, :name, :quantity)");
        query.bindValue(":meal_id", mealId);
        query.bindValue(":name", ingredient.first);
        query.bindValue(":quantity", ingredient.second);
        if (!query.exec()) {
            qDebug() << "Error saving meal ingredient:" << query.lastError().text();
            return false;
        }
    }
    return true;
}

bool DatabaseManager::loadMeals(int userId, QMap<int, QList<MealPlanView::MealInfo>> &meals)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    meals.clear();
    QSqlQuery mealQuery;
    mealQuery.prepare("SELECT id, day_of_week, name, time, calories, image_path FROM meals WHERE user_id = :user_id");
    mealQuery.bindValue(":user_id", userId);

    if (mealQuery.exec()) {
        while (mealQuery.next()) {
            int mealId = mealQuery.value(0).toInt();
            int dayOfWeek = mealQuery.value(1).toInt();
            MealPlanView::MealInfo meal;
            meal.name = mealQuery.value(2).toString();
            meal.time = mealQuery.value(3).toString();
            meal.calories = QString("%1 kcal").arg(mealQuery.value(4).toInt());
            meal.image = mealQuery.value(5).toString();

            // Load ingredients
            QSqlQuery ingredientQuery;
            ingredientQuery.prepare("SELECT ingredient_name, quantity FROM meal_ingredients WHERE meal_id = :meal_id");
            ingredientQuery.bindValue(":meal_id", mealId);
            if (ingredientQuery.exec()) {
                while (ingredientQuery.next()) {
                    meal.ingredients.append({ingredientQuery.value(0).toString(), ingredientQuery.value(1).toString()});
                }
            } else {
                qDebug() << "Error loading meal ingredients:" << ingredientQuery.lastError().text();
                return false;
            }

            meals[dayOfWeek].append(meal);
        }
        return true;
    }
    qDebug() << "Error loading meals:" << mealQuery.lastError().text();
    return false;
}

bool DatabaseManager::saveExercise(int userId, int dayOfWeek, const QString &name,
                                   const QString &duration, int calories, bool completed)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;

    // Vérifier si l'exercice existe déjà
    query.prepare("SELECT id FROM exercises WHERE user_id = :user_id AND day_of_week = :day AND name = :name");
    query.bindValue(":user_id", userId);
    query.bindValue(":day", dayOfWeek);
    query.bindValue(":name", name);

    if (query.exec() && query.next()) {
        // UPDATE de l'exercice existant
        int exerciseId = query.value(0).toInt();
        query.prepare("UPDATE exercises SET duration = :duration, calories = :calories, completed = :completed WHERE id = :id");
        query.bindValue(":duration", duration);
        query.bindValue(":calories", calories);
        query.bindValue(":completed", completed ? 1 : 0);
        query.bindValue(":id", exerciseId);
    } else {
        // INSERT nouvel exercice
        query.prepare("INSERT INTO exercises (user_id, day_of_week, name, duration, calories, completed) "
                      "VALUES (:user_id, :day, :name, :duration, :calories, :completed)");
        query.bindValue(":user_id", userId);
        query.bindValue(":day", dayOfWeek);
        query.bindValue(":name", name);
        query.bindValue(":duration", duration);
        query.bindValue(":calories", calories);
        query.bindValue(":completed", completed ? 1 : 0);
    }

    if (!query.exec()) {
        qDebug() << "Error saving exercise:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::loadExercises(int userId, QMap<int, QList<MealPlanView::ExerciseInfo>> &exercises)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    exercises.clear();
    QSqlQuery query;
    query.prepare("SELECT day_of_week, name, duration, calories, completed FROM exercises WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            int dayOfWeek = query.value(0).toInt();
            MealPlanView::ExerciseInfo exercise;
            exercise.name = query.value(1).toString();
            exercise.duration = query.value(2).toString();
            exercise.calories = QString("%1 kcal").arg(query.value(3).toInt());
            exercise.completed = query.value(4).toBool();
            exercises[dayOfWeek].append(exercise);
        }
        return true;
    }
    qDebug() << "Error loading exercises:" << query.lastError().text();
    return false;
}
bool DatabaseManager::saveWaterData(int userId, const QString &date, int dailyGoal, int currentAmount)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO water_intake (user_id, date, daily_goal, current_amount) "
                  "VALUES (:user_id, :date, :daily_goal, :current_amount)");
    query.bindValue(":user_id", userId);
    query.bindValue(":date", date);
    query.bindValue(":daily_goal", dailyGoal);
    query.bindValue(":current_amount", currentAmount);

    if (!query.exec()) {
        qDebug() << "Error saving water data:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::loadWaterData(int userId, const QString &date, int &dailyGoal, int &currentAmount)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT daily_goal, current_amount FROM water_intake WHERE user_id = :user_id AND date = :date");
    query.bindValue(":user_id", userId);
    query.bindValue(":date", date);

    if (query.exec() && query.next()) {
        dailyGoal = query.value(0).toInt();
        currentAmount = query.value(1).toInt();
        return true;
    }
    return false; // No data found for the user and date
}
bool DatabaseManager::cleanupOrphanedData()
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;

    // Nettoyer les ingrédients orphelins
    if (!query.exec("DELETE FROM meal_ingredients WHERE meal_id NOT IN (SELECT id FROM meals)")) {
        qDebug() << "Error cleaning orphaned ingredients:" << query.lastError().text();
        return false;
    }

    qDebug() << "Cleaned up orphaned data successfully";
    return true;
}

// Fonction pour vérifier la cohérence des données
bool DatabaseManager::verifyDataIntegrity(int userId)
{
    if (!isOpen() && !openDatabase()) {
        return false;
    }

    QSqlQuery query;

    // Vérifier les meals sans ingrédients
    query.prepare("SELECT COUNT(*) FROM meals m WHERE m.user_id = :user_id AND "
                  "NOT EXISTS (SELECT 1 FROM meal_ingredients mi WHERE mi.meal_id = m.id)");
    query.bindValue(":user_id", userId);

    if (query.exec() && query.next()) {
        int mealsWithoutIngredients = query.value(0).toInt();
        if (mealsWithoutIngredients > 0) {
            qDebug() << "Warning: Found" << mealsWithoutIngredients << "meals without ingredients for user" << userId;
        }
    }

    // Vérifier les ingrédients orphelins
    if (query.exec("SELECT COUNT(*) FROM meal_ingredients mi WHERE "
                   "NOT EXISTS (SELECT 1 FROM meals m WHERE m.id = mi.meal_id)")) {
        if (query.next()) {
            int orphanedIngredients = query.value(0).toInt();
            if (orphanedIngredients > 0) {
                qDebug() << "Warning: Found" << orphanedIngredients << "orphaned ingredients";
                return false; // Données incohérentes
            }
        }
    }

    return true;
}

// Fonction pour récupérer les données de debug
void DatabaseManager::debugMealData(int userId)
{
    if (!isOpen() && !openDatabase()) {
        return;
    }

    QSqlQuery query;

    qDebug() << "=== DEBUG MEAL DATA FOR USER" << userId << "===";

    // Lister tous les meals
    query.prepare("SELECT id, day_of_week, name, time, calories FROM meals WHERE user_id = :user_id ORDER BY day_of_week, time");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            int mealId = query.value(0).toInt();
            qDebug() << QString("Meal ID: %1, Day: %2, Name: %3, Time: %4, Calories: %5")
                            .arg(mealId).arg(query.value(1).toInt()).arg(query.value(2).toString())
                            .arg(query.value(3).toString()).arg(query.value(4).toInt());

            // Lister les ingrédients pour ce meal
            QSqlQuery ingredientQuery;
            ingredientQuery.prepare("SELECT ingredient_name, quantity FROM meal_ingredients WHERE meal_id = :meal_id");
            ingredientQuery.bindValue(":meal_id", mealId);

            if (ingredientQuery.exec()) {
                while (ingredientQuery.next()) {
                    qDebug() << QString("  - %1: %2").arg(ingredientQuery.value(0).toString())
                    .arg(ingredientQuery.value(1).toString());
                }
            }
        }
    }

    qDebug() << "=== END DEBUG ===";
}
