#include "DisplayTable.h"
#include "PixelDelegate.h"
#include "yidsrom.h"
#include "utils.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <iostream>
#include <sstream>

using namespace std;

DisplayTable::DisplayTable(QWidget* parent,YidsRom* rom) {
    Q_UNUSED(parent);
    this->shouldShowCollision = true;

    this->yidsRom = rom;

    // Layout //
    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(DisplayTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(DisplayTable::CELL_SIZE_PX);
    this->setRowCount(DisplayTable::CELL_COUNT_H);
    this->setColumnCount(DisplayTable::CELL_COUNT_W);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setShowGrid(false);
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable text editing

    // Drag and Drop //
    this->setMouseTracking(true);

    setItemDelegate(new PixelDelegate);

    QTableWidget::connect(this, &QTableWidget::cellEntered, this, &DisplayTable::cellEnteredTriggered);
    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &DisplayTable::displayTableClicked);
}

/**
 * @brief Places a tile on the DisplayTable. Creates if it doesn't exist, updates
 * if it already exists.
 * 
 * @param x X Position of tile
 * @param y Y Position of tile
 * @param pren ChartilePreRenderData
 */
void DisplayTable::putTile(uint32_t x, uint32_t y, ChartilePreRenderData &pren) {
    if (x > (uint32_t)this->columnCount()) {
        std::cerr << "[ERROR] X value too high: " << hex << x << std::endl;
        return;
    }
    if (y > (uint32_t)this->rowCount()) {
        std::cerr << "[ERROR] Y value too high: " << hex << y << std::endl;
        return;
    }
    // if (pren.tileAttr == 0) {
    //     return;
    // }
    const uint32_t pixelTileSize = this->yidsRom->pixelTiles.size();
    if (pixelTileSize < 1) {
        std::cerr << "[ERROR] pixelTiles is empty, cannot place tile" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (pren.tileId >= pixelTileSize) {
        std::cerr << "[ERROR] Tile ID '" << hex << pren.tileId;
        std::cerr << "' is greater than pixelTiles count " << hex << pixelTileSize << std::endl;
        return;
    }
    int pal = (int)pren.paletteId; // int is more commonly used to access, so convert it early
    if (pal > 0xf) {
        cerr << "paletteId unusually high, got " << hex << pal << endl;
        pal = 0;
    }
    auto loadedTile = this->yidsRom->pixelTiles.at(pren.tileId);
    auto potentialExisting = this->item(y,x);
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setData(PixelDelegateData::PIXEL_ARRAY,loadedTile.tiles);
        newItem->setData(PixelDelegateData::PALETTE_ARRAY,this->yidsRom->currentPalettes[pal]);
        newItem->setData(PixelDelegateData::TILEATTR,(uint)pren.tileAttr);
        newItem->setData(PixelDelegateData::FLIP_H,pren.flipH);
        newItem->setData(PixelDelegateData::FLIP_V,pren.flipV);
        newItem->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::CLEAR);
        newItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
        newItem->setData(PixelDelegateData::DEBUG,loadedTile.index);
        //cout << "x: " << hex << x << ", y: " << hex << y << endl;
        this->setItem(y,x,newItem);
    } else {
        // There is already an item here, lets just update it
        potentialExisting->setData(PixelDelegateData::PIXEL_ARRAY,loadedTile.tiles);
        potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY,this->yidsRom->currentPalettes[pal]);
        potentialExisting->setData(PixelDelegateData::TILEATTR,(uint)pren.tileAttr);
        potentialExisting->setData(PixelDelegateData::FLIP_H,pren.flipH);
        potentialExisting->setData(PixelDelegateData::FLIP_V,pren.flipV);
        potentialExisting->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::CLEAR);
        potentialExisting->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
        potentialExisting->setData(PixelDelegateData::DEBUG,loadedTile.index);
    }
}

void DisplayTable::cellEnteredTriggered(int y, int x) {
    //cout << "Cell entered at x,y: " << hex << x << "," << y << endl;
    QTableWidgetItem* curCell = this->item(y,x);
    if (curCell == nullptr) {
        return;
    } else {
        //cout << hex << curCell->data(PixelDelegateData::TILEATTR).toUInt() << endl;
    }
}

void DisplayTable::displayTableClicked(int row, int column) {
    cout << hex << row << "," << hex << column << endl;
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        // Nothing has loaded yet, cancel
        return;
    }
    cout << dec << curCell->data(PixelDelegateData::DEBUG).toInt() << endl;
    //cout << hex << curCell->data(PixelDelegateData::COLLISION_DRAW).toInt() << endl;
}

void DisplayTable::setCellCollision(int row, int column, CollisionDraw colType) {
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        // Nothing has loaded yet, cancel
        return;
    }
    curCell->setData(PixelDelegateData::COLLISION_DRAW,colType);
}

void DisplayTable::initCellCollision() {
    if (this->yidsRom->canvasWidthCol < 1) {
        cerr << "[ERROR] Canvas Width for Collision is invalid! " << endl;
        return;
    }
    const uint32_t cutOff = this->yidsRom->canvasWidthCol/2;
    const int CELL_LIST_SIZE = this->yidsRom->collisionTileArray.size();
    if (CELL_LIST_SIZE < 1) {
        cerr << "[ERROR] Collision Tile Array is empty!" << endl;
        return;
    }
    for (int colIndex = 0; colIndex < CELL_LIST_SIZE; colIndex++) {
        const auto curCol = this->yidsRom->collisionTileArray.at(colIndex);
        uint32_t y = (colIndex / cutOff)*2;
        uint32_t x = (colIndex % cutOff)*2;
        if (curCol == CollisionType::SQUARE) {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
        } else if (curCol == CollisionType::PLATFORM_PASSABLE) {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::ZIG_ZAG);
            this->setCellCollision(y+1,x+1,CollisionDraw::ZIG_ZAG);
        } else if (curCol == CollisionType::DOWN_RIGHT_45) {
            this->setCellCollision(y,  x,  CollisionDraw::DIAG_DOWN_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT);
            this->setCellCollision(y+1,x+1,CollisionDraw::DIAG_DOWN_RIGHT);
        } else if (curCol == CollisionType::UP_RIGHT_45) {
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::DIAG_UP_RIGHT);
            this->setCellCollision(y  ,x+1,CollisionDraw::DIAG_UP_RIGHT);
        } else if (curCol != 0) { // Unknown, draw temp
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
        }
    }
}

void DisplayTable::toggleShowCollision() {
    if (this->shouldShowCollision == true) {
        this->shouldShowCollision = false;
        auto allItems = this->items(nullptr);
    } else {
        this->shouldShowCollision = true;
    }
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                potentialItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
            }
        }
    }
}

void DisplayTable::updateBgs() {
    uint32_t preRenderSize = this->yidsRom->preRenderDataBg2.size();
    if (preRenderSize == 0) {
        std::cerr << "[WARN] preRenderDataBg2 is empty" << std::endl;
        return;
    }
    if (this->yidsRom->canvasWidthBg2 == 0) {
        std::cerr << "[ERROR] Canvas Width was never set!" << std::endl;
        return;
    }
    if (this->yidsRom->pixelTiles.size() < 1) {
        std::cerr << "[ERROR] Cannot updated BG2, missing pixelTiles" << std::endl;
        return;
    }
    //const uint32_t cutOff = 0x10*32.5;
    const uint32_t cutOff = this->yidsRom->canvasWidthBg2;
    for (uint32_t preRenderIndex = 0; preRenderIndex < preRenderSize; preRenderIndex++) {
        uint32_t y = preRenderIndex / cutOff;
        uint32_t x = preRenderIndex % cutOff;
        ChartilePreRenderData curShort = YUtils::getCharPreRender(this->yidsRom->preRenderDataBg2.at(preRenderIndex));
        //cout << "x: " << hex << x << ", y: " << hex << y << endl;
        this->putTile(x,y,curShort);
    }
}

void DisplayTable::updateObjects() {
    if (this->yidsRom->loadedLevelObjects.size() == 0) {
        cout << "[WARN] No objects loaded" << endl;
        return;
    }
    for (auto it = this->yidsRom->loadedLevelObjects.cbegin(); it != this->yidsRom->loadedLevelObjects.cend(); it++) {
        auto x = it->xPosition;
        auto y = it->yPosition;
        auto potentialExisting = this->item(y,x);
        // Very basic pointing to position
        if (potentialExisting == nullptr) {
            // Do nothing, not there
        } else {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_BOTTOM_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_TOP_RIGHT);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_BOTTOM_LEFT);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_TOP_LEFT);
        }
    }
}

int DisplayTable::wipeTable() {
    std::cout << "wipeTable()" << std::endl;
    uint32_t xWidth = this->columnCount();
    uint32_t yHeight = this->rowCount();
    for (uint32_t col = 0; col < xWidth; col++) {
        for (uint32_t row = 0; row < yHeight; row++) {
            auto currentItem = this->item(row,col);
            if (currentItem != nullptr) {
                this->takeItem(row,col);
                delete currentItem;
            }
        }
    }
    return 0;
}