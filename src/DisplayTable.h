#ifndef DISPLAYTABLE_H
#define DISPLAYTABLE_H

#include "Chartile.h"
#include "yidsrom.h"

#include <QtCore>
#include <QTableWidget>

class DisplayTable : public QTableWidget {
    Q_OBJECT
public:
    DisplayTable(QWidget *parent, YidsRom* rom);
    void putTile(uint32_t x, uint32_t y, ChartilePreRenderData &pren);
private:
    const static int CELL_SIZE_PX = 8;
    const static int CELL_COUNT = 0xff*4;
    YidsRom* yidsRom;
};

#endif