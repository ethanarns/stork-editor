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
}

void ChartilesTable::refreshLoadedTiles() {
    std::vector<Chartile>* chartiles;
    chartiles = &this->yidsRom->pixelTilesBg2;
    for (auto it = chartiles->begin(); it != chartiles->end(); it++) {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,it->tiles);
        newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->currentPalettes[0]);
        if (it->tiles.size() != 64) {
            cerr << "Wanted 64 pixels, got " << dec << it->tiles.size() << endl;
            continue;
        }
        newItem->setText(tr(""));
        int y = it->index / 0x10;
        int x = it->index % 0x10;
        if (this->item(y,x) != nullptr) {
            delete this->item(y,x);
        }
        this->setItem(y,x,newItem);
    }
}