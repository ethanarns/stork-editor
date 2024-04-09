#include "ChartilesTable.h"
#include "../yidsrom.h"
#include "../PixelDelegate.h"
#include "../PixelDelegateEnums.h"
#include "../utils.h"
#include "../GlobalSettings.h"

#include <iostream>
#include <map>

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>

ChartilesTable::ChartilesTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(ChartilesTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(ChartilesTable::CELL_SIZE_PX);
    this->setColumnCount(ChartilesTable::CHARTILES_TABLE_WIDTH);
    this->setRowCount(ChartilesTable::CHARTILES_ROW_COUNT_DEFAULT);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setItemDelegate(new PixelDelegate);

    // Drag and Drop //
    this->setMouseTracking(true);

    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &ChartilesTable::chartilesTableClicked);
}

void ChartilesTable::refreshLoadedMapTilesMap(int whichBg) {
    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("refreshLoadedMapTilesMap: mapData is null",DebugType::WARNING);
        return;
    }
    this->whichBgLoaded = whichBg;
    this->wipeTiles();
    std::map<uint32_t,Chartile> tilesMap;
    if (whichBg == 1) {
        auto scen = this->yidsRom->mapData->getScenByBg(1,true);
        if (scen == nullptr) {
            YUtils::printDebug("refreshLoadedMapTilesMap: couldn't get BG 1",DebugType::WARNING);
            return;
        }
        tilesMap = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    } else if (whichBg == 2) {
        auto scen = this->yidsRom->mapData->getScenByBg(2,true);
        if (scen == nullptr) {
            YUtils::printDebug("refreshLoadedMapTilesMap: couldn't get BG 2",DebugType::WARNING);
            return;
        }
        tilesMap = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    } else {
        auto scen = this->yidsRom->mapData->getScenByBg(3,true);
        if (scen == nullptr) {
            YUtils::printDebug("refreshLoadedMapTilesMap: couldn't get BG 3",DebugType::WARNING);
            return;
        }
        tilesMap = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    }
    uint32_t mapSize = tilesMap.size();
    this->setRowCount((mapSize / 16)+1);
    uint32_t indexForOffset = 0;
    for (uint32_t mapIndex = 0; mapIndex < mapSize; mapIndex++) {
        auto chartile = tilesMap[mapIndex];
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,chartile.tiles);
        newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[this->paletteIndex]);
        newItem->setData(PixelDelegateData::FLIP_H_BG1,false);
        newItem->setData(PixelDelegateData::FLIP_V_BG1,false);
        newItem->setData(PixelDelegateData::TILE_ID_BG1,mapIndex);
        newItem->setData(PixelDelegateData::DRAW_BG1,true);
        newItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
        uint32_t x = indexForOffset % 0x10;
        uint32_t y = indexForOffset / 0x10;
        if (this->item(y,x) != nullptr) {
            delete this->item(y,x);
        }
        this->setItem(y,x,newItem);
        indexForOffset++;
    }
}

void ChartilesTable::wipeTiles() {
    for (int tileRowIndex = 0; tileRowIndex < this->rowCount(); tileRowIndex++) {
        for (int tileColumnIndex = 0; tileColumnIndex < this->columnCount(); tileColumnIndex++) {
            if (this->item(tileRowIndex,tileColumnIndex) != nullptr) {
                this->takeItem(tileRowIndex,tileColumnIndex);
                delete this->item(tileRowIndex,tileColumnIndex);
            }
        }
    }
}

void ChartilesTable::paletteValueChanged(int i) {
    //YUtils::printDebug("paletteValueChanged",DebugType::VERBOSE);
    if (whichBgLoaded < 1) {
        YUtils::printDebug("Cannot load bg index 0",DebugType::ERROR);
        return;
    }
    //std::cout << std::hex << i << std::endl;
    this->paletteIndex = (uint32_t)i;
    globalSettings.currentPaletteIndex = (uint32_t)i;
    this->refreshLoadedMapTilesMap(whichBgLoaded);
}

void ChartilesTable::chartilesTableClicked(int row, int column) {
    std::cout << "Row: " << std::hex << row << ", column: " << std::hex << column << std::endl;
    auto potentialItem = this->item(row,column);
    if (potentialItem == nullptr) {
        std::cout << "No item in location" << std::endl;
    } else {
        std::cout << "Item in location" << std::endl;
        uint32_t tileId = potentialItem->data(PixelDelegateData::TILE_ID_BG1).toUInt();
        auto tileArray = potentialItem->data(PixelDelegateData::PIXEL_ARRAY_BG1).toByteArray();
        std::cout << "Tile ID: 0x" << std::hex << tileId << std::endl;
        std::vector<uint8_t> printableArray(tileArray.begin(), tileArray.end());
        YUtils::printVector(printableArray);
        globalSettings.currentTileIndex = tileId;
    }
}
