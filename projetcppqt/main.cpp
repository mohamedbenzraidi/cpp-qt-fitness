// main.cpp
#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include "loginwindow.h"
#include "dashboardwindow.h"

QPixmap createTransparentIcon(const QString& imagePath, int size = 64) {
    // Charger l'image originale
    QPixmap original(imagePath);

    // Créer une nouvelle image transparente
    QPixmap transparent(size, size);
    transparent.fill(Qt::transparent);

    QPainter painter(&transparent);
    painter.setRenderHint(QPainter::Antialiasing);

    // Rendre le fond blanc transparent
    QPixmap source = original.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Créer un masque pour enlever le blanc
    QBitmap mask = source.createMaskFromColor(QColor(255, 255, 255), Qt::MaskInColor);
    source.setMask(mask);

    // Dessiner l'image sans fond blanc
    painter.drawPixmap(0, 0, source);

    return transparent;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // AJOUT: Définir l'icône de l'application sans fond blanc
    QPixmap iconPixmap = createTransparentIcon(":/images/mon_icone.png");
    a.setWindowIcon(QIcon(iconPixmap));

    // Application du style moderne
    a.setStyle(QStyleFactory::create("Fusion"));

    // Palette de couleurs et style global
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(248, 249, 250));
    palette.setColor(QPalette::WindowText, QColor(43, 45, 66));
    palette.setColor(QPalette::Button, QColor(255, 255, 255));
    palette.setColor(QPalette::ButtonText, QColor(43, 45, 66));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(238, 238, 238));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    palette.setColor(QPalette::ToolTipText, QColor(43, 45, 66));
    palette.setColor(QPalette::Text, QColor(43, 45, 66));
    palette.setColor(QPalette::Highlight, QColor(76, 201, 240));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    a.setPalette(palette);

    // Affichage de la fenêtre de connexion
    LoginWindow w;
    w.show();

    return a.exec();
}
