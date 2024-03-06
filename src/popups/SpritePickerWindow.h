#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QString>

class SpritePickerWindow : public QWidget {
    Q_OBJECT
public:
    SpritePickerWindow(QWidget *parent, YidsRom* rom);
    QListWidget* spriteList;
    QLineEdit* searchBox;

    void updateSpriteList(QString filter);
    void searchTextChanged(const QString &text);
    void currentSpriteChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    YidsRom *yidsRom;
};