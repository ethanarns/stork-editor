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

    this->updateTable(); // TODO: move this elsewhere to avoid issues
}

void BrushTable::updateTable() {
    YUtils::printDebug("updateTable");
    for (int y = 0; y < this->rowCount(); y++) {
        for (int x = 0; x < this->columnCount(); x++) {
            auto potentialExisting = this->item(y,x);
            if (potentialExisting == nullptr) {
                QTableWidgetItem *newItem = new QTableWidgetItem();
                newItem->setData(PixelDelegateData::DRAW_BG1,true);
                newItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[0]);
                this->setItem(y,x,newItem);
            } else {

            }
        }
    }
}

void BrushTable::mousePressEvent(QMouseEvent *event) {
    if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER)
    {
        if (globalSettings.currentTileIndex == 0xffff) {
            YUtils::printDebug("currentTileIndex is unset",DebugType::WARNING);
            return;
        }
        auto item = this->itemAt(event->pos());
        if (item == nullptr) {
            YUtils::printDebug("Failed to get itemAt",DebugType::WARNING);
            return;
        }
        std::cout << "Updating tiles" << std::endl;
        std::map<uint32_t,Chartile> tilesMap = this->yidsRom->mapData->getScenByBg(2)->getVramChartiles();
        item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(globalSettings.currentTileIndex).tiles);
        item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[globalSettings.currentPaletteIndex]);
        item->setData(PixelDelegateData::FLIP_H_BG1,false);
        item->setData(PixelDelegateData::FLIP_V_BG1,false);
        item->setData(PixelDelegateData::TILE_ID_BG1,globalSettings.currentTileIndex);
        item->setData(PixelDelegateData::DRAW_BG1,true);
        item->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
        return;
    }
    QTableWidget::mousePressEvent(event);
}
