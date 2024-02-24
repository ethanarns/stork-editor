#ifndef CHARTILESTABLE_H
#define CHARTILESTABLE_H

#include "../yidsrom.h"

#include <QtCore>
#include <QTableWidget>

class ChartilesTable : public QTableWidget {
    Q_OBJECT
public:
    ChartilesTable(QWidget *parent, YidsRom* rom);
    void refreshLoadedObjectTilesMap();
    void refreshLoadedMapTilesMap(int whichBg);
    void chartilesTableClicked(int row, int column);
    void wipeTiles();
private:
    const static int CELL_SIZE_PX = 16;
    const static int CHARTILES_TABLE_WIDTH = 0x10;
    const static int CHARTILES_ROW_COUNT_DEFAULT = 0x10;
    YidsRom* yidsRom;
};
#endif