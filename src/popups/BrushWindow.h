#pragma once

#include "../yidsrom.h"
#include "BrushTable.h"

#include <QWidget>

class BrushWindow : public QWidget {
    Q_OBJECT
public:
    BrushWindow(QWidget *parent, YidsRom* rom);
    BrushTable* brushTable;
private:
    YidsRom* yidsRom;
};