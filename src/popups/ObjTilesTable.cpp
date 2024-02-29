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
    this->setColumnCount(0x2);
    this->setRowCount(0x2);
    this->setRowCount(ObjTilesTable::OBJTILES_ROW_COUNT_DEFAULT);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setItemDelegate(new PixelDelegate);

    // Drag and Drop //
    this->setMouseTracking(true);

    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &ObjTilesTable::tableClicked);
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
    if (archiveFileName.empty() || archiveFileName.compare("---") == 0) {
        YUtils::printDebug("Invalid sprite file name load attempt");
        return;
    }
    this->currentFileName = archiveFileName;
    std::vector<uint8_t> fileVectorObjset = this->yidsRom->getByteVectorFromFile(archiveFileName);
    auto obarData = new ObjectRenderArchive(fileVectorObjset);
    this->currentObar = obarData;
    this->objbIndex = 0;
    this->frameIndex = 0;
    this->refreshWithCurrentData();
}

void ObjTilesTable::objbValueChanged(int i) {
    //YUtils::printDebug("objbValueChanged()");
    if (this->currentFileName.empty() || this->currentFileName.compare("---") == 0) {
        YUtils::printDebug("Invalid sprite file name in objbValueChanged",DebugType::WARNING);
        return;
    }
    if (this->currentObar == nullptr) {
        YUtils::printDebug("No OBAR loaded",DebugType::WARNING);
        return;
    }
    auto objbs = this->currentObar->objectTileDataVector;
    uint32_t newObjbIndex = (uint32_t)i;
    if (newObjbIndex >= objbs.size()) {
        YUtils::printDebug("objbIndex overflow in objbValueChanged",DebugType::WARNING);
        newObjbIndex = objbs.size() - 1; // Set to last index
        return;
    }
    this->objbIndex = newObjbIndex;
    this->frameIndex = 0;
    this->refreshWithCurrentData();
}

void ObjTilesTable::frameValueChanged(int i) {
    //YUtils::printDebug("frameValueChanged()");
    if (this->currentFileName.empty() || this->currentFileName.compare("---") == 0) {
        YUtils::printDebug("Invalid sprite file name in frameValueChanged");
        return;
    }
    if (this->currentObar == nullptr) {
        YUtils::printDebug("No OBAR loaded",DebugType::WARNING);
        return;
    }
    auto frameCount = this->currentObar->objectTileDataVector.at(this->objbIndex)->frames.size();
    if ((uint32_t)i >= frameCount) {
        YUtils::printDebug("frameIndex overflow in frameValueChanged",DebugType::WARNING);
        this->frameIndex = frameCount - 1;
        return;
    } else {
        this->frameIndex = (uint32_t)i;
    }
    auto curFrame = this->currentObar->objectTileDataVector.at(this->objbIndex)->frames.at(this->frameIndex);
    std::cout << "Metadata for frame 0x" << std::hex << this->frameIndex << ":" << std::endl;
    std::cout << "  " << curFrame->toString() << std::endl;
    std::cout << "  " << curFrame->buildFrame->toString() << std::endl;
    std::cout << "  ObjFrameBuild metadata: 0x";
    uint32_t frameMetaOffset = curFrame->buildFrame->flags & 0b11111;
    frameMetaOffset *= 3;
    uint32_t frameMetaBaseAddress = 0x020D'5E88;
    uint32_t frameMetaAddress = frameMetaBaseAddress + frameMetaOffset;
    uint32_t trueMetaAddress = YUtils::conv2xAddrToFileAddr(frameMetaAddress);
    uint32_t item1 = (uint32_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+0);
    uint32_t item2 = (uint32_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+1);
    uint32_t item3 = (uint32_t)this->yidsRom->uncompedRomVector.at(trueMetaAddress+2);
    std::cout << std::hex << item1 << ", 0x" << item2 << std::hex << ", 0x" << item3 << std::endl;
    this->refreshWithCurrentData();
}

void ObjTilesTable::refreshWithCurrentData(bool guessTileCount) {
    YUtils::printDebug("refreshWithCurrentData()",DebugType::VERBOSE);
    if (guessTileCount) {
        std::cout << "Guessing tile count" << std::endl;
    }
    if (this->currentObar == nullptr) {
        YUtils::printDebug("No OBAR loaded in refreshWithCurrentData",DebugType::WARNING);
        return;
    }
    auto objbs = this->currentObar->objectTileDataVector;
    if (this->objbIndex >= objbs.size()) {
        YUtils::printDebug("objbIndex overflow in refreshWithCurrentData",DebugType::WARNING);
        this->objbIndex = 0;
    }
    auto curObjb = objbs.at(this->objbIndex);
    if (this->frameIndex >= curObjb->frames.size()) {
        YUtils::printDebug("frameIndex overflow in refreshWithCurrentData",DebugType::WARNING);
        this->frameIndex = 0;
    }
    auto curFrame = curObjb->getFrameData(this->frameIndex);
    auto tileCount = 18;
    if (guessTileCount) {
        tileCount = this->getTileCount(curFrame->buildFrame->flags);
    } else {
        tileCount = this->rowCount() * this->columnCount();
    }
    auto tiles = curObjb->getChartiles(curFrame->buildFrame->tileOffset << 4,tileCount);
    if (guessTileCount) {
        this->setRowCount(tiles.size() / this->columnCount());
    }
    this->wipeTiles();
    if (this->currentPalette == nullptr) {
        std::cout << "Setting to default" << std::endl;
        this->currentPalette = this->yidsRom->backgroundPalettes[0];
    }
    for (uint tilesIndex = 0; tilesIndex < tiles.size(); tilesIndex++) {
        auto objChartile = tiles.at(tilesIndex);
        auto tileItem = new QTableWidgetItem();
        tileItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,objChartile);
        tileItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->currentPalette);
        tileItem->setData(PixelDelegateData::FLIP_H_BG1,false);
        tileItem->setData(PixelDelegateData::FLIP_V_BG1,false);
        tileItem->setData(PixelDelegateData::DEBUG_DATA,0x69);
        tileItem->setData(PixelDelegateData::DRAW_OBJECTS,true);
        tileItem->setData(PixelDelegateData::DRAW_BG1,true);
        uint32_t x = tilesIndex % this->columnCount();
        uint32_t y = tilesIndex / this->columnCount();
        if (this->item(y,x) != nullptr) {
            delete this->item(y,x);
        }
        this->setItem(y,x,tileItem);
    }
}

void ObjTilesTable::widthChanged(int i) {
    //std::cout << "widthChanged" << std::endl;
    if (i < 1) {
        YUtils::printDebug("Sprite width selected too small",DebugType::ERROR);
        return;
    }
    this->setColumnCount(i);
    this->refreshWithCurrentData(false);
}

void ObjTilesTable::heightChanged(int i) {
    //std::cout << "widthChanged" << std::endl;
    if (i < 1) {
        YUtils::printDebug("Sprite height selected too small",DebugType::ERROR);
        return;
    }
    this->setRowCount(i);
    this->refreshWithCurrentData(false);
}

void ObjTilesTable::paletteChanged(int i) {
    //std::cout << "paletteChanged: 0x" << std::hex << i << std::endl;
    if (i < -1) {
        YUtils::printDebug("Sprite palette selected too low",DebugType::ERROR);
        return;
    }
    if (i == -1) {
        YUtils::printDebug("Setting to default bg palette");
        this->currentPalette = this->yidsRom->backgroundPalettes[0];
        this->refreshWithCurrentData();
        return;
    }
    auto paletteDataList = this->currentObar->objectPaletteDataVector;
    auto size = paletteDataList.size(); // Can be 0
    //std::cout << "paletteDataSize: 0x" << std::hex << size << std::endl;
    if ((uint32_t)i >= size) {
        YUtils::printDebug("i too big in paletteChanged",DebugType::WARNING);
        return;
    }
    auto curPaletteData = paletteDataList.at(i);
    //std::cout << "Getting first QByteArray..." << std::endl;
    this->currentPalette = curPaletteData->palettes.at(0);
    this->refreshWithCurrentData();
}

// Does not work properly
uint32_t ObjTilesTable::getTileCount(uint32_t buildFlags) {
    // MOSTLY works
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
