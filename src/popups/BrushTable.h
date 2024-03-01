#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QTableWidget>

class BrushTable : public QTableWidget {
    Q_OBJECT
public:
    BrushTable(QWidget *parent, YidsRom* rom);
    void updateTable();
private:
    YidsRom* yidsRom;
    const static int CELL_SIZE_PX = 8;
    const static int CELL_COUNT_DIMS = 12;
};