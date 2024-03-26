#include "BrushTable.h"
#include "../PixelDelegate.h"
#include "../PixelDelegateEnums.h"
#include "../GlobalSettings.h"

#include <QHeaderView>
#include <QMouseEvent>

BrushTable::BrushTable(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(BrushTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(BrushTable::CELL_SIZE_PX);
    this->setRowCount(BrushTable::CELL_COUNT_DIMS);
    this->setColumnCount(BrushTable::CELL_COUNT_DIMS);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setShowGrid(true);
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable text editing
    this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

    setItemDelegate(new PixelDelegate);

    this->resetTable();
}

void BrushTable::resetTable() {
    //YUtils::printDebug("resetTable");
    for (int y = 0; y < this->rowCount(); y++) {
        for (int x = 0; x < this->columnCount(); x++) {
            auto potentialExisting = this->item(y,x);
            if (potentialExisting != nullptr) {
                delete potentialExisting;
            }
            QTableWidgetItem *newItem = new QTableWidgetItem();
            newItem->setData(PixelDelegateData::DRAW_BG1,true);
            newItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
            this->setItem(y,x,newItem);
        }
    }
}

void BrushTable::loadTilesToCurBrush() {
    YUtils::printDebug("Loading brush window tiles to current brush");
    globalSettings.currentBrush->tileAttrs.clear();
    for (int y = 0; y < this->rowCount(); y++) {
        for (int x = 0; x < this->columnCount(); x++) {
            auto item = this->item(y,x);
            MapTileRecordData mapTile;
            if (item != nullptr) {
                mapTile.tileId = item->data(PixelDelegateData::TILE_ID_BG1).toUInt();
                mapTile.paletteId = (uint8_t)item->data(PixelDelegateData::PALETTE_ID_BG1).toUInt();
                mapTile.flipH = item->data(PixelDelegateData::FLIP_H_BG1).toBool();
                mapTile.flipV = item->data(PixelDelegateData::FLIP_V_BG1).toBool();
                mapTile.tileAttr = mapTile.compile();
            } else {
                mapTile.tileId = 0xffff;
                mapTile.paletteId = 0;
                mapTile.flipH = false;
                mapTile.flipV = false;
                mapTile.tileAttr = 0xffff;
            }
            globalSettings.currentBrush->tileAttrs.push_back(mapTile);
        }
    }
    if ((this->rowCount() * this->columnCount()) != (int)globalSettings.currentBrush->tileAttrs.size()) {
        std::stringstream ssMismatch;
        ssMismatch << "Mismatch in row/column mult and current brush data size: 0x";
        ssMismatch << std::hex << (this->rowCount() * this->columnCount()) << " vs 0x";
        ssMismatch << std::hex << globalSettings.currentBrush->tileAttrs.size();
        YUtils::printDebug(ssMismatch.str(),DebugType::ERROR);
        YUtils::popupAlert(ssMismatch.str());
    }
    if (globalSettings.currentBrush->tileAttrs.size() == 0) {
        YUtils::printDebug("No tiles loaded to brush",DebugType::ERROR);
        YUtils::popupAlert("No tiles loaded to brush");
    }
    this->updateBrushDims();
}

void BrushTable::updateBrushDims() {
    globalSettings.brushH = 2;
    globalSettings.brushW = 2;
    for (int y = 0; y < this->rowCount(); y++) {
        for (int x = 0; x < this->columnCount(); x++) {
            auto item = this->item(y,x);
            if (item != nullptr) {
                auto tileId = item->data(PixelDelegateData::TILE_ID_BG1);
                if (!tileId.isNull() && tileId.toInt() != 0xffff && tileId.toInt() != 0x0000) {
                    // Okay there's a good tile
                    if ((x+1) > globalSettings.brushW) {
                        globalSettings.brushW = (x+1);
                    }
                    if ((y+1) > globalSettings.brushH) {
                        globalSettings.brushH = (y+1);
                    }
                }
            }
        }
    }
    // std::cout << globalSettings.brushW << std::endl;
    // std::cout << globalSettings.brushH << std::endl;
}

void BrushTable::setTile(int row, int column, MapTileRecordData tile) {
    auto item = this->item(row,column);
    if (item == nullptr) {
        YUtils::printDebug("Failed to get item for setTile",DebugType::ERROR);
        return;
    }
    
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    std::map<uint32_t,Chartile> tilesMap = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    auto palOffset = this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    uint8_t pal = tile.paletteId + palOffset;
    //std::cout << "Updating tile on BrushTable" << std::endl;
    item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(tile.tileId).tiles);
    item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[pal]);
    item->setData(PixelDelegateData::PALETTE_ID_BG1,pal);
    item->setData(PixelDelegateData::FLIP_H_BG1,tile.flipH);
    item->setData(PixelDelegateData::FLIP_V_BG1,tile.flipV);
    item->setData(PixelDelegateData::TILE_ID_BG1,tile.tileId);
}

void BrushTable::mousePressEvent(QMouseEvent *event) {
    if (globalSettings.currentEditingBackground == 0) {
        YUtils::printDebug("currentEditingBackground is 0, can't edit",DebugType::WARNING);
        YUtils::popupAlert("Select which background to edit from the main window dropdown");
        return;
    }
    if (globalSettings.currentTileIndex == 0xffff) {
        YUtils::printDebug("currentTileIndex is unset (0xffff)",DebugType::WARNING);
        YUtils::popupAlert("Select a tile and palette from BG Tiles Window");
        return;
    }
    auto item = this->itemAt(event->pos());
    if (item == nullptr) {
        YUtils::printDebug("Failed to get itemAt",DebugType::ERROR);
        return;
    }
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    std::map<uint32_t,Chartile> tilesMap = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    //std::cout << "Updating tile on BrushTable" << std::endl;
    item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(globalSettings.currentTileIndex).tiles);
    item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[globalSettings.currentPaletteIndex]);
    item->setData(PixelDelegateData::PALETTE_ID_BG1,globalSettings.currentPaletteIndex);
    item->setData(PixelDelegateData::FLIP_H_BG1,globalSettings.brushFlipH);
    item->setData(PixelDelegateData::FLIP_V_BG1,globalSettings.brushFlipV);
    item->setData(PixelDelegateData::TILE_ID_BG1,globalSettings.currentTileIndex);

    this->loadTilesToCurBrush();
}
