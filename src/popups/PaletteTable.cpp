#include "PaletteTable.h"
#include "../yidsrom.h"
#include "../Chartile.h"
#include "../PixelDelegate.h"
#include "../utils.h"

#include <iostream>
#include <sstream>

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>

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

    this->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable text editing
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setItemDelegate(new PixelDelegate);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setMouseTracking(true);

    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &PaletteTable::paletteTableClicked);
}

void PaletteTable::refreshLoadedTiles() {
    for (uint paletteIndex = 0; paletteIndex < 0x20; paletteIndex++) {
        QByteArray curPalette = this->yidsRom->backgroundPalettes[paletteIndex];
        for (uint colorIndex = 0; colorIndex < PaletteTable::PALETTE_TABLE_WIDTH; colorIndex++) {
            QByteArray fill;
            fill.resize(64);
            for (int i = 0; i < 64; i++) {
                fill[i] = colorIndex;
            }
            auto tileItem = this->item(paletteIndex,colorIndex);
            if (tileItem == nullptr) {
                tileItem = new QTableWidgetItem();
                tileItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,fill);
                tileItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,curPalette);
                tileItem->setData(PixelDelegateData::DRAW_BG2,true);
                this->setItem(paletteIndex,colorIndex,tileItem);
            } else {
                tileItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,fill);
                tileItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,curPalette);
                tileItem->setData(PixelDelegateData::DRAW_BG2,true);
            }
        }
    }
}

void PaletteTable::paletteTableClicked(int row, int column) {
    auto tileItem = this->item(row,column);
    if (tileItem == nullptr) {
        YUtils::printDebug("Palette tile selected has no data",DebugType::WARNING);
        return;
    }

    // Color WORD
    uint8_t firstByte = this->yidsRom->backgroundPalettes[row][column*2];
    uint8_t secondByte = this->yidsRom->backgroundPalettes[row][column*2+1];
    uint16_t colorBytes = (secondByte << 8) + firstByte;
    std::stringstream ssColorShort;
    ssColorShort << std::setfill('0') << std::setw(4) << std::hex << colorBytes;
    std::string colorValueString = YUtils::getUppercase(ssColorShort.str());

    // Set label
    auto label = this->parent()->findChild<QLabel*>("label_colorShort");
    std::stringstream ss;
    uint16_t index = column + (row * 16);
    ss << "Data: " << colorValueString << ", Index: 0x" << std::hex << index;
    uint32_t ramAddress = 0x05000000 + (index * 0x2);
    ss << ", RAM address: " << ramAddress;
    label->setText(ss.str().c_str());
}