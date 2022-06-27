#ifndef DISPLAYTABLE_H
#define DISPLAYTABLE_H

#include "Chartile.h"
#include "yidsrom.h"
#include "PixelDelegate.h"

#include <QtCore>
#include <QTableWidget>

class DisplayTable : public QTableWidget {
    Q_OBJECT
public:
    DisplayTable(QWidget *parent, YidsRom* rom);
    void putTile(uint32_t x, uint32_t y, ChartilePreRenderData &pren);
    void displayTableClicked(int row, int column);
    void setCellCollision(int row, int column, CollisionDraw colType);
    void updateBg2();
    void initCellCollision();
    void toggleShowCollision();
private:
    const static int CELL_SIZE_PX = 8;
    const static int CELL_COUNT_W = 0xff*4;
    const static int CELL_COUNT_H = 0xff*2;
    YidsRom* yidsRom;
    bool shouldShowCollision;

    void cellEnteredTriggered(int row, int column);
};

#endif