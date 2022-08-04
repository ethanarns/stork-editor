#include "ChartilesTable.h"
#include "yidsrom.h"
#include "Chartile.h"
#include "PixelDelegate.h"

#include <iostream>

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>

using namespace std;

ChartilesTable::ChartilesTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(ChartilesTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(ChartilesTable::CELL_SIZE_PX);
    this->setColumnCount(ChartilesTable::CHARTILES_TABLE_WIDTH);
    this->setRowCount(0x100);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setItemDelegate(new PixelDelegate);

    // Drag and Drop //
    this->setMouseTracking(true);

    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &ChartilesTable::chartilesTableClicked);
}

void ChartilesTable::refreshLoadedTiles() {
    std::vector<Chartile>* chartiles;
    chartiles = &this->yidsRom->pixelTilesBg2;
    int _loadedTilesCount = 0;
    for (auto it = chartiles->begin(); it != chartiles->end(); it++) {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        auto tiles = it->tiles;

        // for (int i = 0; i < 64; i++) {
        //     std::cout << hex << (int)tiles[i] << ",";
        // }
        // std::cout << std::endl;

        // Stick it on both to avoid slash marks
        newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tiles);
        newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->currentPalettes[0]);
        newItem->setData(PixelDelegateData::FLIP_H_BG1,false);
        newItem->setData(PixelDelegateData::FLIP_V_BG1,false);
        newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,tiles);
        newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->currentPalettes[0]);
        newItem->setData(PixelDelegateData::FLIP_H_BG2,false);
        newItem->setData(PixelDelegateData::FLIP_V_BG2,false);
        if (tiles.size() != 64) {
            cerr << "Wanted 64 pixels, got " << dec << tiles.size() << std::endl;
            continue;
        }
        newItem->setText(tr(""));
        int y = it->index / 0x10;
        int x = it->index % 0x10;
        if (this->item(y,x) != nullptr) {
            delete this->item(y,x);
        }
        //std::cout << "Placing item at y: " << hex << y << ", x: " << hex << x << std::endl;
        this->setItem(y,x,newItem);
        _loadedTilesCount++;
    }
    Q_UNUSED(_loadedTilesCount); // Do something with this eventually
    //std::cout << "Loaded tiles via refreshLoadedtiles: " << _loadedTilesCount << std::endl;
}

void ChartilesTable::chartilesTableClicked(int row, int column) {
    std::cout << "Row: " << hex << row << ", column: " << hex << column << std::endl;
    auto potentialItem = this->item(row,column);
    if (potentialItem == nullptr) {
        std::cout << "No item in location" << std::endl;
    } else {
        std::cout << "Item in location" << std::endl;
    }
}