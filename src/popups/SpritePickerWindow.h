#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>

class SpritePickerWindow : public QWidget {
    Q_OBJECT
public:
    SpritePickerWindow(QWidget *parent, YidsRom* rom);
    QListWidget* spriteList;
    QLineEdit* searchBox;
private:
    YidsRom *yidsRom;
};