#include "ObjTilesTable.h"
#include "../yidsrom.h"
#include "../Chartile.h"
#include "../PixelDelegate.h"
#include "../utils.h"
#include "../data/ObjectRenderFile.h"

#include <iostream>
#include <string>
#include <sstream>

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>

ObjTilesTable::ObjTilesTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(ObjTilesTable::OBJTILES_CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(ObjTilesTable::OBJTILES_CELL_SIZE_PX);
    this->setColumnCount(ObjTilesTable::OBJTILES_TABLE_WIDTH);
    this->setRowCount(ObjTilesTable::OBJTILES_ROW_COUNT_DEFAULT);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setItemDelegate(new PixelDelegate);

    // Drag and Drop //
    this->setMouseTracking(true);

    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &ObjTilesTable::tableClicked);
}

/// @brief Deprecated function
/// @param fullFileName The file name
/// @deprecated
void ObjTilesTable::loadObjectTiles(std::string fullFileName) {
    YUtils::printDebug("Loading object tiles to the sprite tile popup",DebugType::VERBOSE);
    return;
    auto objectFile = this->yidsRom->getObjPltFile(fullFileName);
    this->wipeTiles();
    // The following (messily) spits out all available tiles for debug purposes
    auto tilesMap = &objectFile.objectPixelTiles;
    uint32_t mapSize = tilesMap->size();
    uint32_t yOffset = 0;
    uint32_t indexForOffset = 0;
    this->setRowCount(ObjTilesTable::OBJTILES_ROW_COUNT_DEFAULT);
    for (uint32_t mapIndex = 0; mapIndex < mapSize; mapIndex++) {
        auto chartilesVector = (*tilesMap)[mapIndex];
        const uint32_t chartilesVectorSize = chartilesVector.size();
        this->setRowCount(this->rowCount()+(chartilesVectorSize/32/0x10));
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
            if (objectFile.objectPalettes.size() > 0) {
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,objectFile.objectPalettes.at(0).paletteData);
            } else {
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[0]);
            }
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

void ObjTilesTable::tableClicked(int row, int column) {
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

void ObjTilesTable::wipeTiles() {
    for (int tileRowIndex = 0; tileRowIndex < this->rowCount(); tileRowIndex++) {
        for (int tileColumnIndex = 0; tileColumnIndex < this->columnCount(); tileColumnIndex++) {
            if (this->item(tileRowIndex,tileColumnIndex) != nullptr) {
                this->takeItem(tileRowIndex,tileColumnIndex);
                delete this->item(tileRowIndex,tileColumnIndex);
            }
        }
    }
}

void ObjTilesTable::doFileLoad(const QString text) {
    YUtils::printDebug("doFileLoad()");
    auto archiveFileName = text.toStdString();
    std::vector<uint8_t> fileVectorObjset = this->yidsRom->getByteVectorFromFile(archiveFileName);
    auto obarData = new ObjectRenderArchive(fileVectorObjset);
    // this->loadObjectTiles(archiveFileName);
    this->wipeTiles();
    uint32_t itemIndex = 0;
    this->setRowCount(0x1000);
    uint32_t offset = 0;
    uint32_t yOffset = 0;
    std::cout << "doing render loop" << std::endl;
    for (uint obarTileIndex = 0; obarTileIndex < obarData->objectTileDataVector.size(); obarTileIndex++) {
        auto objb = obarData->objectTileDataVector.at(obarTileIndex);
        for (uint frameIndex = 0; frameIndex < objb->frames.size(); frameIndex++) {
            auto frame = objb->getFrameData(frameIndex);
            auto tileCount = this->getTileCountMaybe(frame->buildFrame->flags);
            auto tiles = objb->getChartiles(frame->buildFrame->tileOffset << 4,tileCount);
            for (uint tilesIndex = 0; tilesIndex < tiles.size(); tilesIndex++) {
                auto objChartile = tiles.at(tilesIndex);
                auto tileItem = new QTableWidgetItem();
                tileItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,objChartile);
                tileItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[0]);
                tileItem->setData(PixelDelegateData::FLIP_H_BG1,false);
                tileItem->setData(PixelDelegateData::FLIP_V_BG1,false);
                tileItem->setData(PixelDelegateData::DEBUG_DATA,0x69);
                tileItem->setData(PixelDelegateData::DRAW_OBJECTS,true);
                tileItem->setData(PixelDelegateData::DRAW_BG1,true);
                uint32_t x = (itemIndex+offset) % 0x10;
                uint32_t y = (itemIndex+offset) / 0x10 + yOffset;
                if (this->item(y,x) != nullptr) {
                    delete this->item(y,x);
                }
                this->setItem(y,x,tileItem);
                itemIndex++;
            }
            offset++;
        }
        yOffset++;
    }
    std::cout << "doFileLoad done" << std::endl;
}

uint32_t ObjTilesTable::getTileCountMaybe(uint32_t buildFlags) {
    uint32_t frameMetaOffset = buildFlags & 0b11111;
    frameMetaOffset *= 3;
    uint32_t frameMetaBaseAddress = 0x020D'5E88;
    uint32_t frameMetaAddress = frameMetaBaseAddress + frameMetaOffset;
    //std::cout << "0x" << std::hex << frameMetaAddress << std::endl;
    uint32_t trueMetaAddress = YUtils::conv2xAddrToFileAddr(frameMetaAddress);
    //std::cout << "0x" << std::hex << (uint16_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+0) << std::endl;
    //std::cout << "0x" << std::hex << (uint16_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+1) << std::endl;
    //std::cout << "0x" << std::hex << (uint16_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+2) << std::endl;
    return (uint32_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+0);
}