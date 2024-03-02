#include "BrushTable.h"
#include "../PixelDelegate.h"
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

    setItemDelegate(new PixelDelegate);

    this->resetTable();
}

void BrushTable::resetTable() {
    YUtils::printDebug("resetTable");
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
    std::map<uint32_t,Chartile> tilesMap = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground)->getVramChartiles();
    //std::cout << "Updating tile on BrushTable" << std::endl;
    item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(globalSettings.currentTileIndex).tiles);
    item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[globalSettings.currentPaletteIndex]);
    item->setData(PixelDelegateData::FLIP_H_BG1,globalSettings.brushFlipH);
    item->setData(PixelDelegateData::FLIP_V_BG1,globalSettings.brushFlipV);
    item->setData(PixelDelegateData::TILE_ID_BG1,globalSettings.currentTileIndex);
}
