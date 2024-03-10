#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>
#include <QComboBox>

class LevelWindow : public QWidget {
public:
    LevelWindow(QWidget *parent, YidsRom* rom);
    QListWidget* entranceListWidget;
    QListWidget* exitListWidget;
    void refreshLists();
private:
    YidsRom* yidsRom;

    QComboBox* musicIdDropdown;

    QComboBox* entranceAnim;
    QComboBox* entranceScreen;

    QComboBox* exitTypeCombo;
    QComboBox* exitMapTarget;
    QComboBox* exitEntranceTarget;
};