#ifndef DISPLAYTABLE_H
#define DISPLAYTABLE_H

#include "Chartile.h"
#include "yidsrom.h"
#include "PixelDelegate.h"
#include "LevelObject.h"

#include <QtCore>
#include <QTableWidget>

enum LayerSelectMode {
    BG1_LAYER,
    BG2_LAYER,
    BG3_LAYER,
    SPRITES_LAYER,
    COLLISION_LAYER
};

class DisplayTable : public QTableWidget {
    Q_OBJECT
public:
    bool drawBg1;
    bool drawBg2;
    bool drawBg3;
    bool drawObjects;
    bool shouldShowCollision;

    LayerSelectMode layerSelectMode = LayerSelectMode::SPRITES_LAYER;

    DisplayTable(QWidget *parent, YidsRom* rom);
    void putTileBg(uint32_t x, uint32_t y, ChartilePreRenderData &pren, uint16_t whichBg);
    void placeObjectTile(
        uint32_t x, uint32_t y,
        uint32_t objectOffset,
        uint32_t subTile,
        uint32_t paletteOffset,
        uint32_t spriteWidth,
        uint32_t tilesLength,
        ObjectFileName paletteFile,
        ObjectFileName objectFile,
        uint32_t xPixelOffset,
        uint32_t yPixelOffset,
        uint32_t uuid
    );
    void displayTableClicked(int row, int column);
    void setCellCollision(int row, int column, CollisionDraw colType);
    void updateBg();
    void initCellCollision();
    void updateShowCollision();
    void setLayerDraw(int whichLayer, bool shouldDraw);
    void updateObjects();
    int wipeTable();
    void selectItemByUuid(int uuid);
private:
    const static int CELL_SIZE_PX = 8;
    const static int CELL_COUNT_W = 0xff*4;
    const static int CELL_COUNT_H = 0xff*2;
    YidsRom* yidsRom;

    void cellEnteredTriggered(int row, int column);
};

#endif