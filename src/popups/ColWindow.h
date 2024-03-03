#pragma once

#include "../yidsrom.h"
#include "ColTable.h"

#include <QWidget>

class ColWindow : public QWidget {
    Q_OBJECT
public:
    ColWindow(QWidget *parent, YidsRom* rom);
    ColTable* table;
private:
    YidsRom *yidsRom;
};