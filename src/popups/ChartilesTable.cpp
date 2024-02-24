#include "ChartilesTable.h"
#include "../yidsrom.h"
#include "../Chartile.h"
#include "../PixelDelegate.h"
#include "../utils.h"

#include <iostream>

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

void ChartilesTable::refreshLoadedObjectTilesMap() {
    this->wipeTiles();
    // The following (messily) spits out all available tiles for debug purposes
    auto tilesMap = &this->yidsRom->objectFiles[ObjectFileName::OBJSET].objectPixelTiles;
    uint32_t mapSize = tilesMap->size();
    uint32_t yOffset = 0;
    uint32_t indexForOffset = 0;
    for (uint32_t mapIndex = 0; mapIndex < mapSize; mapIndex++) {
        auto chartilesVector = (*tilesMap)[mapIndex];
        const uint32_t chartilesVectorSize = chartilesVector.size();
        for (uint32_t i = 0; i < chartilesVectorSize; i += Constants::CHARTILE_DATA_SIZE) {
            QTableWidgetItem *newItem = new QTableWidgetItem();
            const uint32_t start = i;
            uint32_t end = i + Constants::CHARTILE_DATA_SIZE;
            if (end > chartilesVectorSize) {
                indexForOffset++;
                // This is likely it trying to get crap from other areas
                //cout << "End too big: " << std::hex << end << ", versus size: " << std::hex << chartilesVectorSize << endl;
                continue;
            }
            auto currentSubSection = YUtils::subVector(chartilesVector,start,end);
            auto qArray = YUtils::tileVectorToQByteArray(currentSubSection);
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,qArray);
            newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[0]);
            newItem->setData(PixelDelegateData::FLIP_H_BG1,false);
            newItem->setData(PixelDelegateData::FLIP_V_BG1,false);
            newItem->setData(PixelDelegateData::DEBUG_DATA,mapIndex);
            newItem->setData(PixelDelegateData::DRAW_OBJECTS,true);
            newItem->setData(PixelDelegateData::DRAW_BG1,true);
            uint32_t x = indexForOffset % 0x10;
            uint32_t y = indexForOffset / 0x10 + yOffset;
            if (this->item(y,x) != nullptr) {
                delete this->item(y,x);
            }
            this->setItem(y,x,newItem);
            indexForOffset++;
        }
        yOffset += 2;
    }
}

void ChartilesTable::refreshLoadedMapTilesMap(int whichBg) {
    std::cout << "refreshLoadedMapTilesMap" << std::endl;
    this->wipeTiles();
    std::map<uint32_t,Chartile> tilesMap;
    if (whichBg == 1) {
        tilesMap = this->yidsRom->mapData->getScenByBg(1)->getVramChartiles();
    } else if (whichBg == 2) {
        tilesMap = this->yidsRom->mapData->getScenByBg(2)->getVramChartiles();
    } else {
        tilesMap = this->yidsRom->mapData->getScenByBg(3)->getVramChartiles();;
    }
    uint32_t mapSize = tilesMap.size();
    this->setRowCount((mapSize / 16)+1);
    uint32_t indexForOffset = 0;
    for (uint32_t mapIndex = 0; mapIndex < mapSize; mapIndex++) {
        auto chartile = tilesMap[mapIndex];
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,chartile.tiles);
        newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[0]);
        newItem->setData(PixelDelegateData::FLIP_H_BG1,false);
        newItem->setData(PixelDelegateData::FLIP_V_BG1,false);
        newItem->setData(PixelDelegateData::DEBUG_DATA,mapIndex);
        newItem->setData(PixelDelegateData::DRAW_BG1,true);
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

void ChartilesTable::chartilesTableClicked(int row, int column) {
    std::cout << "Row: " << std::hex << row << ", column: " << std::hex << column << std::endl;
    auto potentialItem = this->item(row,column);
    if (potentialItem == nullptr) {
        std::cout << "No item in location" << std::endl;
    } else {
        std::cout << "Item in location" << std::endl;
        uint32_t foundDebug = potentialItem->data(PixelDelegateData::DEBUG_DATA).toUInt();
        auto tileArray = potentialItem->data(PixelDelegateData::PIXEL_ARRAY_BG1).toByteArray();
        std::cout << "Index: " << std::hex << foundDebug << std::endl;
        std::vector<uint8_t> printableArray(tileArray.begin(), tileArray.end());
        YUtils::printVector(printableArray);
    }
}