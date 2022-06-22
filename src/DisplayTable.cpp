#include "DisplayTable.h"
#include "PixelDelegate.h"
#include "yidsrom.h"

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
    this->setRowCount(DisplayTable::CELL_COUNT);
    this->setColumnCount(DisplayTable::CELL_COUNT);
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

void DisplayTable::putTile(uint32_t x, uint32_t y, ChartilePreRenderData &pren) {
    if (x > DisplayTable::CELL_COUNT) {
        cerr << "X value too high: " << hex << x << endl;
        return;
    }
    if (y > DisplayTable::CELL_COUNT) {
        cerr << "Y value too high: " << hex << y << endl;
        return;
    }
    if (pren.tileAttr == 0) {
        return;
    }
    int pal = (int)pren.paletteId; // int is more commonly used to access, so convert it early
    if (pal > 0xf) {
        cerr << "paletteId unusually high, got " << hex << pal << endl;
        pal = 0;
    }
    auto loadedTile = this->yidsRom->pixelTiles.at(pren.tileId);
    QTableWidgetItem *newItem = new QTableWidgetItem();
    newItem->setData(PixelDelegateData::PIXEL_ARRAY,loadedTile.tiles);
    newItem->setData(PixelDelegateData::PALETTE_ARRAY,this->yidsRom->currentPalettes[pal]);
    newItem->setData(PixelDelegateData::TILEATTR,(uint)pren.tileAttr);
    newItem->setData(PixelDelegateData::FLIP_H,pren.flipH);
    newItem->setData(PixelDelegateData::FLIP_V,pren.flipV);
    newItem->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::CLEAR);
    newItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
    this->setItem(y,x,newItem);
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
    const uint32_t cutOff = 520/2;
    const int CELL_LIST_SIZE = this->yidsRom->collisionTileArray.size();
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