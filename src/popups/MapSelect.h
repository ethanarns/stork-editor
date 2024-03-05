#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>

class MapSelect : public QWidget {
    Q_OBJECT
public:
    MapSelect(QWidget *parent, YidsRom* rom);
    YidsRom* yidsRom;
    void updateLeftList();
private:
    QListWidget* leftList;
    QListWidget* rightList;
};