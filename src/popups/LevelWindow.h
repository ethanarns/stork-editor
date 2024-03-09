#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>

class LevelWindow : public QWidget {
public:
    LevelWindow(QWidget *parent, YidsRom* rom);
    QListWidget* entranceListWidget;
    QListWidget* exitListWidget;
private:
    YidsRom* yidsRom;
};