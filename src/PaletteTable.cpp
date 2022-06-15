#include "PaletteTable.h"
#include "yidsrom.h"
#include "Chartile.h"
#include "PixelDelegate.h"

#include <iostream>

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>

using namespace std;

PaletteTable::PaletteTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(PaletteTable::CELL_SIZE_WIDTH_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(PaletteTable::CELL_SIZE_HEIGHT_PX);
    this->setColumnCount(PaletteTable::PALETTE_TABLE_WIDTH);
    this->setRowCount(PaletteTable::PALETTE_TABLE_HEIGHT);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setItemDelegate(new PixelDelegate);

    // Drag and Drop //
    this->setMouseTracking(true);
}

void PaletteTable::refreshLoadedTiles() {
    // Probably do something unique here

    // int killStop = 0x100;
    // int killIndex = 0;
    // for (auto it = this->yidsRom->pixelTiles.begin(); it != this->yidsRom->pixelTiles.end(); it++) {
    //     QTableWidgetItem *newItem = new QTableWidgetItem();
    //     newItem->setData(PixelDelegateData::PIXEL_ARRAY,it->tiles);
    //     newItem->setData(PixelDelegateData::PALETTE_ARRAY,this->yidsRom->currentPalettes[0]);
    //     if (it->tiles.size() != 64) {
    //         cerr << "Wanted 64 pixels, got " << dec << it->tiles.size() << endl;
    //         continue;
    //     }
    //     newItem->setText(tr(""));
    //     int y = it->index / 0x10;
    //     int x = it->index % 0x10;
    //     this->setItem(y,x,newItem);
    //     killIndex++;
    //     if (killIndex >= killStop) {
    //         return;
    //     }
    // }
}