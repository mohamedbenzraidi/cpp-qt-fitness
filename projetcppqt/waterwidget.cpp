#include "waterwidget.h"
#include <QDate>
#include <QMessageBox>
#include <QFontDatabase>
#include <QPainter>
#include <QStyleOption>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPropertyAnimation>

// ===== WaterGlassButton Implementation =====
WaterGlassButton::WaterGlassButton(const QString &text, int amount, QWidget *parent)
    : QPushButton(parent), m_amount(amount)
{
    setMinimumHeight(100);
    setIconSize(QSize(32, 32));
    QString buttonText = QString("<div align='center'>"
                                 "<span style='font-size:11pt; font-weight:600;'>%1 ml</span><br>"
                                 "<span style='font-size:9pt;'>%2</span>"
                                 "</div>").arg(amount).arg(text);
    setToolTip(QString("Ajouter %1 ml d'eau").arg(amount));
    QLabel* textLabel = new QLabel(buttonText, this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setTextFormat(Qt::RichText);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(textLabel);
    setLayout(layout);
}
// ===== WaterWidget Implementation =====
WaterWidget::WaterWidget(int userId, QWidget *parent)
    : QFrame(parent), m_userId(userId), m_dailyGoal(2000), m_currentAmount(0)
{
    setupUI();
    styleComponents();
    loadData();
    updateUI();
}

void WaterWidget::setupUI()
{
    // Layout principal avec marges pour l'effet d'ombre
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ===== Section de titre et statut =====
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(15, 15, 15, 10);
    headerLayout->setSpacing(5);

    // Titre avec icône d'eau
    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setFixedSize(32, 32);
    iconLabel->setPixmap(QPixmap(":/icons/water.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    if (iconLabel->pixmap().isNull()) {
        // Fallback si l'icône n'est pas disponible
        iconLabel->setText("💧");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("font-size: 24px;");
    }

    m_titleLabel = new QLabel("Suivi d'hydratation", this);
    m_titleLabel->setObjectName("titleLabel");

    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(m_titleLabel, 1);
    headerLayout->addLayout(titleLayout);

    // Status label
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setObjectName("statusLabel");
    headerLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(headerFrame);

    // ===== Section de progression =====
    QFrame *progressFrame = new QFrame(this);
    progressFrame->setObjectName("progressFrame");

    QVBoxLayout *progressLayout = new QVBoxLayout(progressFrame);
    progressLayout->setContentsMargins(15, 15, 15, 15);
    progressLayout->setSpacing(10);

    // Barre de progression
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, m_dailyGoal);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p% - %v ml / %m ml");
    m_progressBar->setMinimumHeight(25);
    m_progressBar->setObjectName("waterProgressBar");

    progressLayout->addWidget(m_progressBar);

    // Affichage de la quantité
    m_amountLabel = new QLabel("0 ml consommés aujourd'hui", this);
    m_amountLabel->setAlignment(Qt::AlignCenter);
    m_amountLabel->setObjectName("amountLabel");
    progressLayout->addWidget(m_amountLabel);

    mainLayout->addWidget(progressFrame);

    // ===== Section de boutons rapides =====
    m_quickAddFrame = new QFrame(this);
    m_quickAddFrame->setObjectName("quickAddFrame");

    QVBoxLayout *quickAddLayout = new QVBoxLayout(m_quickAddFrame);
    quickAddLayout->setContentsMargins(15, 15, 15, 15);
    quickAddLayout->setSpacing(10);

    QLabel *quickAddLabel = new QLabel("Ajout rapide", this);
    quickAddLabel->setObjectName("sectionLabel");
    quickAddLayout->addWidget(quickAddLabel);

    QHBoxLayout *glassButtonsLayout = new QHBoxLayout();
    glassButtonsLayout->setSpacing(10);

    m_smallGlassButton = new WaterGlassButton("Petit verre", 200, this);
    m_mediumGlassButton = new WaterGlassButton("Verre moyen", 300, this);
    m_largeGlassButton = new WaterGlassButton("Grande bouteille", 500, this);

    m_smallGlassButton->setObjectName("smallGlassButton");
    m_mediumGlassButton->setObjectName("mediumGlassButton");
    m_largeGlassButton->setObjectName("largeGlassButton");

    glassButtonsLayout->addWidget(m_smallGlassButton);
    glassButtonsLayout->addWidget(m_mediumGlassButton);
    glassButtonsLayout->addWidget(m_largeGlassButton);

    quickAddLayout->addLayout(glassButtonsLayout);

    mainLayout->addWidget(m_quickAddFrame);

    // ===== Section d'ajout personnalisé =====
    m_customAddFrame = new QFrame(this);
    m_customAddFrame->setObjectName("customAddFrame");

    QVBoxLayout *customAddVLayout = new QVBoxLayout(m_customAddFrame);
    customAddVLayout->setContentsMargins(15, 15, 15, 15);
    customAddVLayout->setSpacing(10);

    QLabel *customAddLabel = new QLabel("Quantité personnalisée", this);
    customAddLabel->setObjectName("sectionLabel");
    customAddVLayout->addWidget(customAddLabel);

    QHBoxLayout *customAddHLayout = new QHBoxLayout();
    customAddHLayout->setSpacing(10);

    m_customSlider = new QSlider(Qt::Horizontal, this);
    m_customSlider->setRange(50, 1000);
    m_customSlider->setValue(250);
    m_customSlider->setTickInterval(50);
    m_customSlider->setTickPosition(QSlider::TicksBelow);
    m_customSlider->setObjectName("customSlider");

    m_customSpinBox = new QSpinBox(this);
    m_customSpinBox->setRange(50, 1000);
    m_customSpinBox->setValue(250);
    m_customSpinBox->setSuffix(" ml");
    m_customSpinBox->setObjectName("customSpinBox");

    m_addCustomButton = new QPushButton("Ajouter", this);
    m_addCustomButton->setObjectName("addCustomButton");

    customAddHLayout->addWidget(m_customSlider, 3);
    customAddHLayout->addWidget(m_customSpinBox, 1);
    customAddHLayout->addWidget(m_addCustomButton, 1);

    customAddVLayout->addLayout(customAddHLayout);

    mainLayout->addWidget(m_customAddFrame);

    // ===== Bouton de réinitialisation =====
    QHBoxLayout *resetLayout = new QHBoxLayout();
    resetLayout->setContentsMargins(0, 5, 0, 0);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_resetButton = new QPushButton("Réinitialiser", this);
    m_resetButton->setObjectName("resetButton");
    m_resetButton->setMaximumWidth(150);

    resetLayout->addWidget(spacer);
    resetLayout->addWidget(m_resetButton);

    mainLayout->addLayout(resetLayout);

    // Animation de la barre de progression
    m_progressAnimation = new QPropertyAnimation(m_progressBar, "value", this);
    m_progressAnimation->setDuration(800);
    m_progressAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Connexions
    connect(m_smallGlassButton, &QPushButton::clicked, this, &WaterWidget::onGlassButtonClicked);
    connect(m_mediumGlassButton, &QPushButton::clicked, this, &WaterWidget::onGlassButtonClicked);
    connect(m_largeGlassButton, &QPushButton::clicked, this, &WaterWidget::onGlassButtonClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &WaterWidget::resetWater);
    connect(m_customSlider, &QSlider::valueChanged, m_customSpinBox, &QSpinBox::setValue);
    connect(m_customSpinBox, &QSpinBox::valueChanged, m_customSlider, &QSlider::setValue);
    connect(m_addCustomButton, &QPushButton::clicked, this, &WaterWidget::onAddButtonClicked);

    // Timer pour sauvegarder automatiquement les données
    QTimer *saveTimer = new QTimer(this);
    connect(saveTimer, &QTimer::timeout, this, [=]() { saveData(); });
    saveTimer->start(60000); // Sauvegarde toutes les minutes
}

QFrame* WaterWidget::createSeparator()
{
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setObjectName("separator");
    return separator;
}

void WaterWidget::styleComponents()
{
    // Style global pour le widget
    setObjectName("waterWidget");

    // Propriétés du frame principal
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);

    // Base Style
    QString baseStyle = QString(
        "#waterWidget {"
        "    background-color: #ffffff;"
        "    border-radius: 10px;"
        "    border: none;"
        "}"

        "#headerFrame {"
        "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1a73e8, stop:1 #5195ee);"
        "    border-radius: 8px;"
        "    color: white;"
        "}"

        "#progressFrame, #quickAddFrame, #customAddFrame {"
        "    background-color: #f8f9fa;"
        "    border-radius: 8px;"
        "    border: 1px solid #e0e0e0;"
        "}"

        "#titleLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: white;"
        "}"

        "#statusLabel {"
        "    font-size: 14px;"
        "    color: white;"
        "    font-weight: bold;"
        "}"

        "#sectionLabel {"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #3c4043;"
        "    padding-bottom: 5px;"
        "    border-bottom: 1px solid #e0e0e0;"
        "}"

        "#amountLabel {"
        "    font-size: 14px;"
        "    color: #5f6368;"
        "}"

        "#waterProgressBar {"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 4px;"
        "    background-color: #f1f3f4;"
        "    height: 20px;"
        "    text-align: center;"
        "    font-weight: bold;"
        "    color: #3c4043;"
        "}"

        "#waterProgressBar::chunk {"
        "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1a73e8, stop:1 #5195ee);"
        "    border-radius: 3px;"
        "}"

        "WaterGlassButton {"
        "    background-color: #e8f0fe;"
        "    border: 1px solid #dadce0;"
        "    border-radius: 8px;"
        "    color: #1a73e8;"
        "    font-weight: bold;"
        "}"

        "WaterGlassButton:hover {"
        "    background-color: #d2e3fc;"
        "    border: 1px solid #1a73e8;"
        "}"

        "WaterGlassButton:pressed {"
        "    background-color: #1a73e8;"
        "    color: white;"
        "}"

        "#customSlider {"
        "    height: 20px;"
        "}"

        "#customSlider::groove:horizontal {"
        "    border: 1px solid #dadce0;"
        "    height: 10px;"
        "    background: #f1f3f4;"
        "    border-radius: 5px;"
        "}"

        "#customSlider::handle:horizontal {"
        "    background: #1a73e8;"
        "    border: 1px solid #1a73e8;"
        "    width: 18px;"
        "    margin: -5px 0;"
        "    border-radius: 9px;"
        "}"

        "#customSpinBox {"
        "    border: 1px solid #dadce0;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "    background-color: white;"
        "    selection-background-color: #1a73e8;"
        "}"

        "#addCustomButton {"
        "    background-color: #1a73e8;"
        "    color: white;"
        "    border-radius: 4px;"
        "    padding: 8px 15px;"
        "    font-weight: bold;"
        "    border: none;"
        "}"

        "#addCustomButton:hover {"
        "    background-color: #1967d2;"
        "}"

        "#addCustomButton:pressed {"
        "    background-color: #185abc;"
        "}"

        "#resetButton {"
        "    background-color: white;"
        "    color: #d93025;"
        "    border: 1px solid #d93025;"
        "    border-radius: 4px;"
        "    padding: 5px 15px;"
        "}"

        "#resetButton:hover {"
        "    background-color: #fce8e6;"
        "}"

        "#resetButton:pressed {"
        "    background-color: #d93025;"
        "    color: white;"
        "}"

        "#separator {"
        "    color: #e0e0e0;"
        "}"
        );

    setStyleSheet(baseStyle);

    // Effet d'ombre pour le widget principal
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 50));
    shadowEffect->setOffset(0, 2);
    this->setGraphicsEffect(shadowEffect);
}

void WaterWidget::updateUI()
{
    m_amountLabel->setText(QString("%1 ml consommés aujourd'hui").arg(m_currentAmount));

    // Mise à jour du statut
    float percentage = static_cast<float>(m_currentAmount) / m_dailyGoal * 100;

    if (percentage < 25) {
        m_statusLabel->setText("Pensez à vous hydrater davantage!");
    } else if (percentage < 50) {
        m_statusLabel->setText("Vous êtes sur la bonne voie!");
    } else if (percentage < 75) {
        m_statusLabel->setText("Continuez comme ça!");
    } else if (percentage < 100) {
        m_statusLabel->setText("Presque à votre objectif!");
    } else {
        m_statusLabel->setText("Félicitations! Objectif atteint!");
    }

    // Animation de la progression
    m_progressAnimation->setStartValue(m_progressBar->value());
    m_progressAnimation->setEndValue(m_currentAmount);
    m_progressAnimation->start();
}

void WaterWidget::setDailyGoal(int milliliters)
{
    if (milliliters > 0) {
        m_dailyGoal = milliliters;
        m_progressBar->setMaximum(m_dailyGoal);
        updateUI();
        saveData();
    }
}
int WaterWidget::consumedWater() const
{
    return m_currentAmount;
}

void WaterWidget::addWater(int milliliters)
{
    if (milliliters > 0) {
        m_currentAmount += milliliters;
        updateUI();
        saveData();
    }
}

void WaterWidget::resetWater()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Réinitialiser le suivi d'eau");
    msgBox.setText("Êtes-vous sûr de vouloir réinitialiser votre consommation d'eau pour aujourd'hui?");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    msgBox.setStyleSheet(
        "QMessageBox {"
        "    background-color: #ffffff;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    background-color: #1a73e8;"
        "    color: white;"
        "    border-radius: 4px;"
        "    padding: 6px 16px;"
        "    font-weight: bold;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1967d2;"
        "}"
        "QPushButton[text=\"Non\"] {"
        "    background-color: white;"
        "    color: #1a73e8;"
        "    border: 1px solid #1a73e8;"
        "}"
        "QPushButton[text=\"Non\"]:hover {"
        "    background-color: #f8f9fa;"
        "}"
        );

    if (msgBox.exec() == QMessageBox::Yes) {
        m_currentAmount = 0;
        updateUI();
        saveData(); // <--- APPEL CORRIGÉ (sans argument)
    }
}
void WaterWidget::updateProgress()
{
    // Cette fonction est maintenant remplacée par l'animation
    m_progressBar->setValue(m_currentAmount);
}

void WaterWidget::onAddButtonClicked()
{
    addWater(m_customSpinBox->value());
}

void WaterWidget::onCustomValueChanged(int value)
{
    m_customSpinBox->setValue(value);
}

void WaterWidget::onGlassButtonClicked()
{
    WaterGlassButton *button = qobject_cast<WaterGlassButton*>(sender());
    if (button) {
        addWater(button->waterAmount());
    }
}

void WaterWidget::saveData()
{
    DatabaseManager::instance().saveWaterData(m_userId, QDate::currentDate().toString(Qt::ISODate), m_dailyGoal, m_currentAmount);
}

// DANS la fonction WaterWidget::loadData()
void WaterWidget::loadData()
{
    if (!DatabaseManager::instance().loadWaterData(m_userId, QDate::currentDate().toString(Qt::ISODate), m_dailyGoal, m_currentAmount)) {
        m_dailyGoal = 2000;
        m_currentAmount = 0;
    }
    m_progressBar->setMaximum(m_dailyGoal);
}
