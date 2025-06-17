#ifndef HABITSVIEW_H
#define HABITSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QFrame>
#include <QCheckBox>
#include <QProgressBar>
#include <QLabel>
#include <QMap>
#include <QSet>
#include <QDate>
#include "databasemanager.h"

struct Habit
{
    QString name;
    int currentStreak;
    int goalDays;
    QSet<QDate> completedDates;
    bool completedToday;

    Habit() : currentStreak(0), goalDays(30), completedToday(false) {}
    Habit(const QString &n, int goal = 30) : name(n), currentStreak(0), goalDays(goal), completedToday(false) {}
};

class HabitsView : public QWidget
{
    Q_OBJECT

public:
  explicit HabitsView(int userId, QWidget *parent = nullptr);

private slots:
    void onDateSelected();
    void onAddHabitClicked();
    void onHabitChecked(bool checked);

private:
    void loadHabitsFromDatabase();
    void saveHabitToDatabase(int habitId);
    void initUI();
    // void loadInitialData();
    void connectSignals();
    void updateStreakDisplay();
    void updateDailyHabitsDisplay();
    void updateCalendarDisplay();
    void updateStatistics();
    void addNewHabit(const QString &name, int goalDays = 30);
    void calculateStreaks();

    // UI Helper methods
    QFrame* createStyledFrame();
    QFrame* createStyledCard(const QString &title, const QString &value, const QString &colorHex);
    QFrame* createHabitCheckItem(const QString &habitName, bool checked, int habitId);
    QTextCharFormat getDateTextFormat(const QColor &color);

    // Data members
    QMap<int, Habit> m_habits;
    int m_nextHabitId;
    QDate m_selectedDate;
     int m_userId;
    // Statistics
    double m_completionRate;
    int m_longestStreak;
    int m_perfectDays;

    // UI components
    QGridLayout *m_streakGrid;
    QVBoxLayout *m_dailyHabitsLayout;
    QCalendarWidget *m_calendar;

    // Statistics cards (for dynamic updates)
    QLabel *m_completionRateLabel;
    QLabel *m_totalHabitsLabel;
    QLabel *m_longestStreakLabel;
    QLabel *m_perfectDaysLabel;
};

#endif // HABITSVIEW_H
