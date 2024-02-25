#pragma once

#include "../yidsrom.h"

#include <QtCore>
#include <QTableWidget>

class ObjTilesTable : public QTableWidget {
    Q_OBJECT
public:
    ObjTilesTable(QWidget *parent, YidsRom* rom);
    void loadObjectTiles(std::string fullFileName);
    void tableClicked(int row, int column);
    void wipeTiles();
    void doFileLoad(const QString text);
private:
    const static int OBJTILES_CELL_SIZE_PX = 16;
    const static int OBJTILES_TABLE_WIDTH = 0x10;
    const static int OBJTILES_ROW_COUNT_DEFAULT = 0x1;
    YidsRom* yidsRom;
};