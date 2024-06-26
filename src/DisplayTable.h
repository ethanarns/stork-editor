#ifndef DISPLAYTABLE_H
#define DISPLAYTABLE_H

#include <vector>
#include <cstdint>

#include "Chartile.h"
#include "yidsrom.h"
#include "PixelDelegate.h"
#include "PixelDelegateEnums.h"
#include "LevelObject.h"
#include "GlobalSettings.h"

#include <QtCore>
#include <QTableWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QByteArray>
#include <QPoint>
#include <QRubberBand>
#include <QRect>
#include <QUndoCommand>

class DisplayTable : public QTableWidget {
    Q_OBJECT
public:
    bool drawBg1;
    bool drawBg2;
    bool drawBg3;
    bool drawObjects;
    bool shouldShowCollision;
    bool shouldShowTriggers;
    bool shouldDrawEntrances;
    bool shouldDrawExits;
    bool shouldDrawBlkz;
    /**
     * A list/vector of uint32_t UUIDs of selected objects, usually LevelObjects
    */
    std::vector<uint32_t> selectedObjects;
    /// @brief Layer drawing options are present on tiles by default, but you don't
    // want to do a mass enable outside the user's consent more than once
    bool firstLayerDrawDone = false;

    DisplayTable(QWidget *parent, YidsRom* rom);
    void putTileBg(uint32_t x, uint32_t y, MapTileRecordData &pren, uint16_t whichBg);
    void placeObjectGraphic(
        uint32_t x, uint32_t y,
        uint32_t objectOffset,
        uint32_t frame,
        uint32_t paletteOffset,
        std::string paletteFile,
        std::string objectFile,
        int manualXoffset,
        int manualYoffset,
        uint32_t uuid,
        bool isLz10,
        bool forceFlipH, bool forceFlipV,
        bool isExtra = false
    );
    void setCellCollision(int row, int column, CollisionDraw colType, uint8_t _colDebug);
    void updateBg();
    void initCellCollision();
    void updateShowCollision();
    void updateTriggerBoxes(int highlight = -1);
    void updateBlkzLayer();
    void setLayerDraw(LayerShouldDraw whichLayer, bool shouldDraw);
    void updateSprites();
    int wipeTable();
    
    //void wipeObject(uint32_t uuid);
    void selectItemByUuid(uint32_t uuid,bool triggerMainWindowUpdate = true);
    void moveSpriteTo(uint32_t uuid, uint32_t newX, uint32_t newY);
    /// @brief This both updates the grid as well as updates the MapData
    /// @param row Y location on grid and MapData
    /// @param column X location on grid and MapData
    /// @param pren BG Map tile data struct
    /// @return true if place successful, false if failed
    bool placeNewTileOnMap(int row, int column, MapTileRecordData mapRecord, uint32_t whichBg, bool skipPalOffset = true);
    void clearVisualSpriteSelection();
    void updatePortals(bool drawEntrances, bool drawExits);
    void updateSelectedTilesVisuals(int whichBg);
private:
    const static int CELL_COUNT_W = 0xff*4;
    const static int CELL_COUNT_H = 0xff*2;
    YidsRom* yidsRom;

    void cellEnteredTriggered(int row, int column);
    void printCellDebug(QTableWidgetItem *item, uint whichBg);
    QPoint getTopLeftOfSprite(uint32_t levelObjectUuid);
    void updateSurrounding(int row, int column, int distance);
    void setHover(int row, int column, HoverType hoverType);

    void doBgBrushClick(QTableWidgetItem* curItem);
    std::vector<QTableWidgetItem*> getIntersectedTiles(QRect selectionRect);
    void handleSpritesRightClickPress(QMouseEvent *event);

    void handleCollisionMouseClick(QPoint position, bool isCopy = false);
protected:
    QPoint dragStartPosition;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    QRubberBand* selectorBand;
    QPoint selectorBandOrigin;
signals:
    int triggerMainWindowUpdate();
    void updateMainWindowStatus(std::string newStatus);
    void pushStateCommandToStack(QUndoCommand* cmdPtr);
    void clipboardUpdated();
};

#endif