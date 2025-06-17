// waterwidget.h
#ifndef WATERWIDGET_H
#define WATERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QPropertyAnimation>
#include "databasemanager.h"

class WaterGlassButton : public QPushButton
{
    Q_OBJECT

public:
    WaterWidget(int userId, QWidget *parent = nullptr);
    explicit WaterGlassButton(const QString &text, int amount, QWidget *parent = nullptr);
    int waterAmount() const { return m_amount; }

private:
    int m_amount;

};

class WaterWidget : public QFrame
{
    Q_OBJECT

public:
     explicit WaterWidget(int userId, QWidget *parent = nullptr);
    void setDailyGoal(int milliliters);
    int consumedWater() const;
    void saveData();
    void loadData();

public slots:
    void addWater(int milliliters);
    void resetWater();

private slots:
    void updateProgress();
    void onAddButtonClicked();
    void onCustomValueChanged(int value);
    void onGlassButtonClicked();

private:
      int m_userId;
    int m_dailyGoal;
    int m_currentAmount;

    // UI Components
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QLabel *m_amountLabel;

    QFrame *m_quickAddFrame;
    WaterGlassButton *m_smallGlassButton;
    WaterGlassButton *m_mediumGlassButton;
    WaterGlassButton *m_largeGlassButton;

    QFrame *m_customAddFrame;
    QSlider *m_customSlider;
    QSpinBox *m_customSpinBox;
    QPushButton *m_addCustomButton;

    QPushButton *m_resetButton;

    QPropertyAnimation *m_progressAnimation;

    void setupUI();
    void styleComponents();
    void updateUI();
    QFrame* createSeparator();
    void createWaveEffect();
};

#endif // WATERWIDGET_H
