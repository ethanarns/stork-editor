#ifndef DISPLAYTABLE_H
#define DISPLAYTABLE_H

#include <vector>

#include "Chartile.h"
#include "yidsrom.h"
#include "PixelDelegate.h"
#include "LevelObject.h"

#include <QtCore>
#include <QTableWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QByteArray>
#include <QPoint>

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
    /**
     * A list/vector of uint32_t UUIDs of selected objects, usually LevelObjects
    */
    std::vector<uint32_t> selectedObjects;
    /// @brief Layer drawing options are present on tiles by default, but you don't
    // want to do a mass enable outside the user's consent more than once
    bool firstLayerDrawDone = false;

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
    void setCellCollision(int row, int column, CollisionDraw colType, uint8_t _colDebug);
    void updateBg();
    void initCellCollision();
    void updateShowCollision();
    void updateTriggerBoxes();
    void setLayerDraw(uint whichLayer, bool shouldDraw);
    void updateSprites();
    int wipeTable();
    
    void wipeObject(uint32_t uuid);
    void selectItemByUuid(uint32_t uuid);
    void moveSpriteTo(uint32_t uuid, uint32_t newX, uint32_t newY);
private:
    const static int CELL_SIZE_PX = 16;
    const static int CELL_COUNT_W = 0xff*4;
    const static int CELL_COUNT_H = 0xff*2;
    YidsRom* yidsRom;

    void cellEnteredTriggered(int row, int column);
    void printCellDebug(QTableWidgetItem *item, uint whichBg);
    QPoint getTopLeftOfSprite(uint32_t levelObjectUuid);
protected:
    QPoint dragStartPosition;
    void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:
    int triggerMainWindowUpdate();
    void updateMainWindowStatus(std::string newStatus);
};

#endif