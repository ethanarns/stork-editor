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
    void displayTableClicked(int row, int column);
    void setCellCollision(int row, int column, CollisionDraw colType);
    void updateBg();
    void initCellCollision();
    void updateShowCollision();
    void updateTriggerBoxes();
    void setLayerDraw(uint whichLayer, bool shouldDraw);
    void updateObjects();
    int wipeTable();
    
    void wipeObject(uint32_t uuid);
    void selectItemByUuid(uint32_t uuid);
    void moveSpriteTo(uint32_t uuid, uint32_t newX, uint32_t newY);
private:
    const static int CELL_SIZE_PX = 8;
    const static int CELL_COUNT_W = 0xff*4;
    const static int CELL_COUNT_H = 0xff*2;
    YidsRom* yidsRom;
    uint32_t currentlyDraggedItem = 0;

    void cellEnteredTriggered(int row, int column);
protected:
    bool dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
signals:
    int triggerMainWindowUpdate();
    void updateMainWindowStatus(std::string newStatus);
};

#endif