#include "PaletteTable.h"
#include "yidsrom.h"
#include "Chartile.h"
#include "PixelDelegate.h"
#include "utils.h"

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
    const int PALETTE_DIMS = 16; // 0x0-0xf

    for (int paletteIndex = 0; paletteIndex < PALETTE_DIMS; paletteIndex++) {
        QByteArray curPalette = this->yidsRom->currentPalettes[paletteIndex];
        for (int colorIndex = 0; colorIndex < PALETTE_DIMS; colorIndex++) {
            // Do not delete this, it is being stored in this class
            // That said, if you are re-setting it, delete the existing one (TODO)
            QTableWidgetItem *newItem = new QTableWidgetItem();
            QByteArray fill;
            fill.resize(64);
            for (int i = 0; i < 64; i++) {
                fill[i] = colorIndex;
            }
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,fill);
            newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,curPalette);
            //cout << "Placing " << hex << colorIndex << " at (" << colorIndex << "," << paletteIndex << ")" << endl;
            this->setItem(paletteIndex,colorIndex,newItem);
        }
        // cout << "Palette " << paletteIndex << endl;
        // for (int testIndex = 0; testIndex < curPalette.size(); testIndex += 2) {
        //     auto tempCol = YUtils::getColorFromBytes(curPalette.at(testIndex),curPalette.at(testIndex+1));
        //     cout << "Color: " << dec << tempCol.red() << "," << tempCol.green() << "," << tempCol.blue() << endl;
        // }
    }
}