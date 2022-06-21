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
    newItem->setData(PixelDelegateData::COLLISIONTYPE,CollisionType::NONE);
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
    curCell->setData(PixelDelegateData::COLLISIONTYPE,CollisionType::SQUARE);
}