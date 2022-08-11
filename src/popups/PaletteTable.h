#ifndef PALETTETABLE_H
#define PALETTETABLE_H

#include "../yidsrom.h"

#include <QtCore>
#include <QTableWidget>

class PaletteTable : public QTableWidget {
    Q_OBJECT
public:
    PaletteTable(QWidget *parent, YidsRom* rom);
    void refreshLoadedTiles();

    const static int PALETTE_TABLE_WINDOW_HEIGHT = 300;
    const static int PALETTE_TABLE_WINDOW_WIDTH = 538;
private:
    const static int CELL_SIZE_HEIGHT_PX = 16;
    const static int CELL_SIZE_WIDTH_PX = 32;
    const static int PALETTE_TABLE_WIDTH = 0x10; // 0x0-0xf
    const static int PALETTE_TABLE_HEIGHT = 0x10; // 0x0-0xf
    YidsRom* yidsRom;
};

#endif