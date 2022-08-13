#include "PaletteTable.h"
#include "../yidsrom.h"
#include "../Chartile.h"
#include "../PixelDelegate.h"
#include "../utils.h"

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
    for (int paletteIndex = 0; paletteIndex < PaletteTable::PALETTE_TABLE_HEIGHT; paletteIndex++) {
        QByteArray curPalette = this->yidsRom->currentPalettes[paletteIndex];
        for (int colorIndex = 0; colorIndex < PaletteTable::PALETTE_TABLE_WIDTH; colorIndex++) {
            auto tileItem = this->item(paletteIndex,colorIndex);
            if (tileItem == nullptr) {
                tileItem = new QTableWidgetItem();
            }
            QByteArray fill;
            fill.resize(64);
            for (int i = 0; i < 64; i++) {
                fill[i] = colorIndex;
            }
            tileItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,fill);
            tileItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,curPalette);
            this->setItem(paletteIndex,colorIndex,tileItem);
        }
    }
}