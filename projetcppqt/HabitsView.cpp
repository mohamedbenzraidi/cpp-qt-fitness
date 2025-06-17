#include "HabitsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFont>
#include <QProgressBar>
#include <QDate>
#include <QCalendarWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QCheckBox>
#include <QFrame>
#include <QColor>
#include <QInputDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
// #include "HabitsView.h"
// #include <QVBoxLayout>
// #include <QHBoxLayout>
// #include <QGridLayout>
// #include <QLabel>
// #include <QFont>
// #include <QProgressBar>
// #include <QDate>
// #include <QCalendarWidget>
// #include <QPushButton>
// #include <QScrollArea>
// #include <QGraphicsDropShadowEffect>
// #include <QCheckBox>
// #include <QFrame>
// #include <QColor>
// #include <QInputDialog>
// #include <QMessageBox>
// #include <QSpinBox>
// #include <QFormLayout>
// #include <QDialog>
// #include <QDialogButtonBox>
// #include <QLineEdit>
// #include <random>
// #include <chrono>

HabitsView::HabitsView(int userId, QWidget *parent)
    : QWidget(parent), m_nextHabitId(1), m_selectedDate(QDate::currentDate()), m_userId(userId)
{
    initUI();
    loadHabitsFromDatabase();
    connectSignals();
    updateStreakDisplay();
    updateDailyHabitsDisplay();
    updateCalendarDisplay();
    updateStatistics();
}

void HabitsView::initUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Header with title
    QLabel *titleLabel = new QLabel("Habits Tracker", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Current streak section
    QFrame *streakFrame = createStyledFrame();
    QVBoxLayout *streakLayout = new QVBoxLayout(streakFrame);

    QLabel *streakTitle = new QLabel("Current Streaks", streakFrame);
    QFont streakFont = streakTitle->font();
    streakFont.setBold(true);
    streakFont.setPointSize(12);
    streakTitle->setFont(streakFont);

    streakLayout->addWidget(streakTitle);

    // Streak grid
    QGridLayout *streakGrid = new QGridLayout();
    streakGrid->setColumnStretch(0, 2);  // Habit name
    streakGrid->setColumnStretch(1, 1);  // Days count
    streakGrid->setColumnStretch(2, 3);  // Progress bar
    streakLayout->addLayout(streakGrid);

    m_streakGrid = streakGrid;
    mainLayout->addWidget(streakFrame);

    // Calendar and daily habits section
    QHBoxLayout *middleLayout = new QHBoxLayout();

    // Calendar on the left
    QFrame *calendarFrame = createStyledFrame();
    QVBoxLayout *calendarLayout = new QVBoxLayout(calendarFrame);

    QLabel *calendarTitle = new QLabel("Activity Calendar", calendarFrame);
    calendarTitle->setFont(streakFont);
    calendarLayout->addWidget(calendarTitle);

    m_calendar = new QCalendarWidget(calendarFrame);
    m_calendar->setMinimumWidth(300);
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setSelectionMode(QCalendarWidget::SingleSelection);

    // Style the calendar
    m_calendar->setStyleSheet(
        "QCalendarWidget QToolButton {"
        "  color: #333333;"
        "  background-color: #f0f0f0;"
        "  border-radius: 4px;"
        "  padding: 5px;"
        "}"
        "QCalendarWidget QMenu {"
        "  background-color: white;"
        "  border: 1px solid #d0d0d0;"
        "}"
        "QCalendarWidget QSpinBox {"
        "  background-color: white;"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 2px;"
        "}"
        "QCalendarWidget QTableView {"
        "  selection-background-color: #4CAF50;"
        "  selection-color: white;"
        "}"
        );

    calendarLayout->addWidget(m_calendar);
    middleLayout->addWidget(calendarFrame);

    // Daily habits on the right
    QFrame *habitsFrame = createStyledFrame();
    QVBoxLayout *habitsLayout = new QVBoxLayout(habitsFrame);

    QLabel *habitsTitle = new QLabel("Today's Habits", habitsFrame);
    habitsTitle->setObjectName("habitsTitle");
    habitsTitle->setFont(streakFont);
    habitsLayout->addWidget(habitsTitle);

    // Create scrollable area for habits
    QScrollArea *habitsScrollArea = new QScrollArea(habitsFrame);
    habitsScrollArea->setWidgetResizable(true);
    habitsScrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *habitsScrollContent = new QWidget(habitsScrollArea);
    m_dailyHabitsLayout = new QVBoxLayout(habitsScrollContent);
    m_dailyHabitsLayout->setAlignment(Qt::AlignTop);
    m_dailyHabitsLayout->setSpacing(10);

    habitsScrollArea->setWidget(habitsScrollContent);
    habitsLayout->addWidget(habitsScrollArea);

    // Add habit button
    QPushButton *addHabitBtn = new QPushButton("Add New Habit", habitsFrame);
    addHabitBtn->setObjectName("addHabitButton");
    addHabitBtn->setCursor(Qt::PointingHandCursor);
    addHabitBtn->setStyleSheet(
        "QPushButton#addHabitButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#addHabitButton:hover {"
        "  background-color: #45a049;"
        "}"
        "QPushButton#addHabitButton:pressed {"
        "  background-color: #3d8b40;"
        "}"
        );

    habitsLayout->addWidget(addHabitBtn);
    middleLayout->addWidget(habitsFrame);

    mainLayout->addLayout(middleLayout);

    // Bottom section for statistics
    QFrame *statsFrame = createStyledFrame();
    QVBoxLayout *statsLayout = new QVBoxLayout(statsFrame);

    QLabel *statsTitle = new QLabel("Habit Statistics", statsFrame);
    statsTitle->setFont(streakFont);
    statsLayout->addWidget(statsTitle);

    QHBoxLayout *statsGridLayout = new QHBoxLayout();

    // Create statistics cards with stored labels for dynamic updates
    QFrame *completionCard = createStyledFrame();
    QVBoxLayout *completionLayout = new QVBoxLayout(completionCard);
    completionLayout->setAlignment(Qt::AlignCenter);
    m_completionRateLabel = new QLabel("0%", completionCard);
    QFont valueFont = m_completionRateLabel->font();
    valueFont.setPointSize(24);
    valueFont.setBold(true);
    m_completionRateLabel->setFont(valueFont);
    m_completionRateLabel->setAlignment(Qt::AlignCenter);
    m_completionRateLabel->setStyleSheet("color: #4CAF50;");
    QLabel *completionTitle = new QLabel("Completion Rate", completionCard);
    completionTitle->setAlignment(Qt::AlignCenter);
    completionTitle->setStyleSheet("color: #555555;");
    completionLayout->addWidget(m_completionRateLabel);
    completionLayout->addWidget(completionTitle);
    statsGridLayout->addWidget(completionCard);

    QFrame *totalCard = createStyledFrame();
    QVBoxLayout *totalLayout = new QVBoxLayout(totalCard);
    totalLayout->setAlignment(Qt::AlignCenter);
    m_totalHabitsLabel = new QLabel("0", totalCard);
    m_totalHabitsLabel->setFont(valueFont);
    m_totalHabitsLabel->setAlignment(Qt::AlignCenter);
    m_totalHabitsLabel->setStyleSheet("color: #2196F3;");
    QLabel *totalTitle = new QLabel("Total Habits", totalCard);
    totalTitle->setAlignment(Qt::AlignCenter);
    totalTitle->setStyleSheet("color: #555555;");
    totalLayout->addWidget(m_totalHabitsLabel);
    totalLayout->addWidget(totalTitle);
    statsGridLayout->addWidget(totalCard);

    QFrame *longestCard = createStyledFrame();
    QVBoxLayout *longestLayout = new QVBoxLayout(longestCard);
    longestLayout->setAlignment(Qt::AlignCenter);
    m_longestStreakLabel = new QLabel("0 days", longestCard);
    m_longestStreakLabel->setFont(valueFont);
    m_longestStreakLabel->setAlignment(Qt::AlignCenter);
    m_longestStreakLabel->setStyleSheet("color: #FF9800;");
    QLabel *longestTitle = new QLabel("Longest Streak", longestCard);
    longestTitle->setAlignment(Qt::AlignCenter);
    longestTitle->setStyleSheet("color: #555555;");
    longestLayout->addWidget(m_longestStreakLabel);
    longestLayout->addWidget(longestTitle);
    statsGridLayout->addWidget(longestCard);

    QFrame *perfectCard = createStyledFrame();
    QVBoxLayout *perfectLayout = new QVBoxLayout(perfectCard);
    perfectLayout->setAlignment(Qt::AlignCenter);
    m_perfectDaysLabel = new QLabel("0", perfectCard);
    m_perfectDaysLabel->setFont(valueFont);
    m_perfectDaysLabel->setAlignment(Qt::AlignCenter);
    m_perfectDaysLabel->setStyleSheet("color: #9C27B0;");
    QLabel *perfectTitle = new QLabel("Perfect Days", perfectCard);
    perfectTitle->setAlignment(Qt::AlignCenter);
    perfectTitle->setStyleSheet("color: #555555;");
    perfectLayout->addWidget(m_perfectDaysLabel);
    perfectLayout->addWidget(perfectTitle);
    statsGridLayout->addWidget(perfectCard);

    statsLayout->addLayout(statsGridLayout);
    mainLayout->addWidget(statsFrame);

    // Set object name for custom styling
    setObjectName("habitsView");
    setStyleSheet(
        "QWidget#habitsView {"
        "  background-color: #f5f5f5;"
        "}"
        );
}

QFrame* HabitsView::createStyledFrame()
{
    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setStyleSheet(
        "QFrame {"
        "  background-color: white;"
        "  border-radius: 8px;"
        "  border: 1px solid #e0e0e0;"
        "}"
        );

    // Add drop shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(frame);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    frame->setGraphicsEffect(shadow);

    return frame;
}

QFrame* HabitsView::createStyledCard(const QString &title, const QString &value, const QString &colorHex)
{
    QFrame *card = new QFrame(this);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(
        "QFrame {"
        "  background-color: white;"
        "  border-radius: 8px;"
        "  border: 1px solid #e0e0e0;"
        "  padding: 10px;"
        "}"
        );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);

    QLabel *valueLabel = new QLabel(value, card);
    QFont valueFont = valueLabel->font();
    valueFont.setPointSize(24);
    valueFont.setBold(true);
    valueLabel->setFont(valueFont);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("color: " + colorHex + ";");

    QLabel *titleLabel = new QLabel(title, card);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(10);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #555555;");

    cardLayout->addWidget(valueLabel);
    cardLayout->addWidget(titleLabel);

    return card;
}

QFrame* HabitsView::createHabitCheckItem(const QString &habitName, bool checked, int habitId)
{
    QFrame *habitFrame = new QFrame(this);
    habitFrame->setStyleSheet(
        "QFrame {"
        "  background-color: #f9f9f9;"
        "  border-radius: 4px;"
        "  border: 1px solid #e0e0e0;"
        "  padding: 10px;"
        "}"
        );

    QHBoxLayout *habitLayout = new QHBoxLayout(habitFrame);
    habitLayout->setContentsMargins(10, 8, 10, 8);

    QCheckBox *checkBox = new QCheckBox(habitName, habitFrame);
    checkBox->setChecked(checked);
    checkBox->setProperty("habitId", habitId);

    // Style the checkbox
    checkBox->setStyleSheet(
        "QCheckBox {"
        "  font-size: 14px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 18px;"
        "  height: 18px;"
        "}"
        "QCheckBox::indicator:unchecked {"
        "  border: 2px solid #bdbdbd;"
        "  background-color: white;"
        "  border-radius: 3px;"
        "}"
        "QCheckBox::indicator:checked {"
        "  border: 2px solid #4CAF50;"
        "  background-color: #4CAF50;"
        "  border-radius: 3px;"
        "}"
        );

    connect(checkBox, &QCheckBox::toggled, this, &HabitsView::onHabitChecked);
    habitLayout->addWidget(checkBox);

    return habitFrame;
}


void HabitsView::addNewHabit(const QString &name, int goalDays)
{
    Habit newHabit(name, goalDays);
    m_habits[m_nextHabitId] = newHabit;
    saveHabitToDatabase(m_nextHabitId);
    m_nextHabitId++;
}

void HabitsView::calculateStreaks()
{
    QDate today = QDate::currentDate();

    for (auto &habit : m_habits) {
        habit.currentStreak = 0;
        habit.completedToday = habit.completedDates.contains(today);

        // Calculer le streak le plus long qui inclut aujourd'hui
        // ou le streak le plus récent si aujourd'hui n'est pas complété

        QDate checkDate = today;

        // Si aujourd'hui n'est pas complété, commencer par hier
        if (!habit.completedDates.contains(today)) {
            checkDate = today.addDays(-1);
        }

        // Compter les jours consécutifs en remontant
        while (habit.completedDates.contains(checkDate)) {
            habit.currentStreak++;
            checkDate = checkDate.addDays(-1);
        }

        // Alternative : Calculer le streak basé sur la date sélectionnée
        // si vous voulez montrer le streak pour une date spécifique
        /*
        QDate selectedDate = m_selectedDate;
        if (habit.completedDates.contains(selectedDate)) {
            habit.currentStreak = 0;
            QDate checkDate = selectedDate;

            // Compter vers le passé
            while (habit.completedDates.contains(checkDate)) {
                habit.currentStreak++;
                checkDate = checkDate.addDays(-1);
            }
        }
        */
    }
}


void HabitsView::updateStreakDisplay()
{
    // Clear existing streak display
    QLayoutItem *child;
    while ((child = m_streakGrid->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // Si aucune habitude, afficher un message
    if (m_habits.isEmpty()) {
        QLabel *emptyLabel = new QLabel("No habits yet. Add your first habit!");
        emptyLabel->setStyleSheet("color: #666666; font-style: italic;");
        m_streakGrid->addWidget(emptyLabel, 0, 0, 1, 3);
        return;
    }

    int row = 0;
    for (auto it = m_habits.begin(); it != m_habits.end(); ++it) {
        const Habit &habit = it.value();

        QLabel *habitLabel = new QLabel(habit.name);
        QFont habitFont = habitLabel->font();
        habitFont.setBold(true);
        habitLabel->setFont(habitFont);

        QLabel *daysLabel = new QLabel(QString::number(habit.currentStreak) + " days");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, habit.goalDays);
        progressBar->setValue(habit.currentStreak);
        progressBar->setTextVisible(true);
        progressBar->setFormat("%v/%m days");

        // Style the progress bar based on progress
        QString progressColor;
        if (habit.currentStreak >= habit.goalDays) {
            progressColor = "#4CAF50"; // Green for completed
        } else if (habit.currentStreak >= habit.goalDays * 0.7) {
            progressColor = "#8BC34A"; // Light green for good progress
        } else if (habit.currentStreak >= habit.goalDays * 0.3) {
            progressColor = "#FFC107"; // Amber for medium progress
        } else {
            progressColor = "#F44336"; // Red for low progress
        }

        progressBar->setStyleSheet(
            "QProgressBar {"
            "  border: 1px solid #e0e0e0;"
            "  border-radius: 5px;"
            "  text-align: center;"
            "  background-color: #f0f0f0;"
            "}"
            "QProgressBar::chunk {"
            "  background-color: " + progressColor + ";"
                              "  border-radius: 4px;"
                              "}"
            );

        m_streakGrid->addWidget(habitLabel, row, 0);
        m_streakGrid->addWidget(daysLabel, row, 1);
        m_streakGrid->addWidget(progressBar, row, 2);

        row++;
    }
}

void HabitsView::updateDailyHabitsDisplay()
{
    // Clear existing habits display
    QLayoutItem *child;
    while ((child = m_dailyHabitsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // Add habits for the selected date
    for (auto it = m_habits.begin(); it != m_habits.end(); ++it) {
        int habitId = it.key();
        const Habit &habit = it.value();

        bool isCompleted = habit.completedDates.contains(m_selectedDate);
        QFrame *habitItem = createHabitCheckItem(habit.name, isCompleted, habitId);
        m_dailyHabitsLayout->addWidget(habitItem);
    }
}

void HabitsView::updateCalendarDisplay()
{
    // Clear existing calendar formatting
    m_calendar->setDateTextFormat(QDate(), QTextCharFormat());

    // Mark completed dates
    for (const auto &habit : m_habits) {
        for (const QDate &date : habit.completedDates) {
            if (date.month() == m_calendar->monthShown() &&
                date.year() == m_calendar->yearShown()) {
                m_calendar->setDateTextFormat(date, getDateTextFormat(Qt::green));
            }
        }
    }

    // Count habits completed per day and use different colors
    QMap<QDate, int> completionCounts;
    for (const auto &habit : m_habits) {
        for (const QDate &date : habit.completedDates) {
            completionCounts[date]++;
        }
    }

    // Apply different colors based on completion ratio
    int totalHabits = m_habits.size();
    if (totalHabits > 0) {
        for (auto it = completionCounts.begin(); it != completionCounts.end(); ++it) {
            QDate date = it.key();
            int completed = it.value();
            double ratio = (double)completed / totalHabits;

            QColor color;
            if (ratio >= 0.9) {
                color = Qt::darkGreen;  // Almost all habits completed
            } else if (ratio >= 0.7) {
                color = Qt::green;      // Most habits completed
            } else if (ratio >= 0.5) {
                color = QColor(255, 165, 0); // Orange - half completed
            } else {
                color = QColor(255, 99, 71);  // Light red - few completed
            }

            m_calendar->setDateTextFormat(date, getDateTextFormat(color));
        }
    }
}

void HabitsView::updateStatistics()
{
    int totalHabits = m_habits.size();
    m_totalHabitsLabel->setText(QString::number(totalHabits));

    if (totalHabits == 0) {
        m_completionRateLabel->setText("0%");
        m_longestStreakLabel->setText("0 days");
        m_perfectDaysLabel->setText("0");
        return;
    }

    // Calculate completion rate (for current week)
    QDate today = QDate::currentDate();
    QDate weekStart = today.addDays(-6);
    int totalPossible = totalHabits * 7;
    int totalCompleted = 0;

    for (int i = 0; i < 7; i++) {
        QDate checkDate = weekStart.addDays(i);
        for (const auto &habit : m_habits) {
            if (habit.completedDates.contains(checkDate)) {
                totalCompleted++;
            }
        }
    }

    m_completionRate = totalPossible > 0 ? (double)totalCompleted / totalPossible * 100 : 0;
    m_completionRateLabel->setText(QString::number((int)m_completionRate) + "%");

    // Find longest streak
    m_longestStreak = 0;
    for (const auto &habit : m_habits) {
        if (habit.currentStreak > m_longestStreak) {
            m_longestStreak = habit.currentStreak;
        }
    }
    m_longestStreakLabel->setText(QString::number(m_longestStreak) + " days");

    // Calculate perfect days (days where all habits were completed)
    QSet<QDate> allDates;
    for (const auto &habit : m_habits) {
        for (const QDate &date : habit.completedDates) {
            allDates.insert(date);
        }
    }

    m_perfectDays = 0;
    for (const QDate &date : allDates) {
        int habitsCompletedOnDate = 0;
        for (const auto &habit : m_habits) {
            if (habit.completedDates.contains(date)) {
                habitsCompletedOnDate++;
            }
        }
        if (habitsCompletedOnDate == totalHabits) {
            m_perfectDays++;
        }
    }
    m_perfectDaysLabel->setText(QString::number(m_perfectDays));
}

QTextCharFormat HabitsView::getDateTextFormat(const QColor &color)
{
    QTextCharFormat format;
    format.setBackground(color);
    format.setForeground(Qt::white);
    return format;
}

void HabitsView::connectSignals()
{
    // Connect calendar date change
    connect(m_calendar, &QCalendarWidget::selectionChanged,
            this, &HabitsView::onDateSelected);

    // Connect add habit button
    QPushButton *addButton = findChild<QPushButton*>("addHabitButton");
    if (addButton) {
        connect(addButton, &QPushButton::clicked, this, &HabitsView::onAddHabitClicked);
    }
}

void HabitsView::onDateSelected()
{
    m_selectedDate = m_calendar->selectedDate();

    // Update the title
    QLabel *titleLabel = findChild<QLabel*>("habitsTitle");
    if (titleLabel) {
        if (m_selectedDate == QDate::currentDate()) {
            titleLabel->setText("Today's Habits");
        } else {
            titleLabel->setText("Habits for " + m_selectedDate.toString("MMM d, yyyy"));
        }
    }

    updateDailyHabitsDisplay();
}

void HabitsView::onAddHabitClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Habit");
    dialog.setFixedSize(300, 150);

    QFormLayout *layout = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Enter habit name");

    QSpinBox *goalSpin = new QSpinBox(&dialog);
    goalSpin->setRange(1, 365);
    goalSpin->setValue(30);
    goalSpin->setSuffix(" days");

    layout->addRow("Habit Name:", nameEdit);
    layout->addRow("Goal (days):", goalSpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addRow(buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        QString habitName = nameEdit->text().trimmed();
        if (!habitName.isEmpty()) {
            addNewHabit(habitName, goalSpin->value());

            // ✅ AJOUT : Calculer les streaks après ajout
            calculateStreaks();

            updateStreakDisplay();
            updateDailyHabitsDisplay();
            updateCalendarDisplay(); // Ajout pour mettre à jour le calendrier
            updateStatistics();
        } else {
            QMessageBox::warning(this, "Invalid Input", "Please enter a habit name.");
        }
    }
}



void HabitsView::onHabitChecked(bool checked)
{
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
    if (!checkBox) return;

    int habitId = checkBox->property("habitId").toInt();
    if (!m_habits.contains(habitId)) return;

    Habit &habit = m_habits[habitId];

    if (checked) {
        habit.completedDates.insert(m_selectedDate);
    } else {
        habit.completedDates.remove(m_selectedDate);
    }

    saveHabitToDatabase(habitId);
    calculateStreaks();
    updateStreakDisplay();
    updateCalendarDisplay();
    updateStatistics();
}
void HabitsView::saveHabitToDatabase(int habitId)
{
    if (!m_habits.contains(habitId)) {
        return;
    }
    DatabaseManager dbManager;
    const Habit &habit = m_habits[habitId];
    dbManager.saveHabit(m_userId, habitId, habit.name, habit.goalDays, habit.completedDates);
}
void HabitsView::loadHabitsFromDatabase()
{
    DatabaseManager dbManager;
    QMap<int, QMap<QString, QVariant>> habits;
    if (dbManager.loadHabits(m_userId, habits)) {
        m_habits.clear();
        m_nextHabitId = dbManager.getNextHabitId(m_userId);
        for (auto it = habits.constBegin(); it != habits.constEnd(); ++it) {
            int habitId = it.key();
            const QMap<QString, QVariant> &data = it.value();
            Habit habit(data["name"].toString(), data["goalDays"].toInt());
            habit.completedDates = data["completedDates"].value<QSet<QDate>>();
            m_habits[habitId] = habit;
        }
    } else {
        // Initialize default habits if none exist
        addNewHabit("Morning Workout", 30);
        addNewHabit("Meditation", 30);
        addNewHabit("Drink Water", 30);
        addNewHabit("Reading", 30);
        for (auto it = m_habits.begin(); it != m_habits.end(); ++it) {
            saveHabitToDatabase(it.key());
        }
    }
    calculateStreaks();
}
