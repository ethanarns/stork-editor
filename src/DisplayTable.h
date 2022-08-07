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
    void putTileBg(uint32_t x, uint32_t y, ChartilePreRenderData &pren, uint16_t whichBg);
    void placeObjectTile(uint32_t x, uint32_t y, uint32_t objectOffset, uint32_t subTile, uint32_t paletteOffset);
    void displayTableClicked(int row, int column);
    void setCellCollision(int row, int column, CollisionDraw colType);
    void updateBg();
    void initCellCollision();
    void toggleShowCollision();
    void updateObjects();
    int wipeTable();
private:
    const static int CELL_SIZE_PX = 8;
    const static int CELL_COUNT_W = 0xff*4;
    const static int CELL_COUNT_H = 0xff*2;
    YidsRom* yidsRom;
    bool shouldShowCollision;

    void cellEnteredTriggered(int row, int column);
};

#endif