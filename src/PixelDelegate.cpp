#ifndef ROMSTRUCTS_H
#define ROMSTRUCTS_H

#include "PixelDelegate.h"

#include <iostream>

#include <QtWidgets>

using namespace std;

// A single drawn tile is 8x8
const int PIXEL_TILE_DIVISIONS = 8;
const int PIXEL_TILE_TOTAL = PIXEL_TILE_DIVISIONS * PIXEL_TILE_DIVISIONS;

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
    //QPen pen;
    //auto byteArray = index.data(PixelDelegateInfo::PIXEL_ARRAY).toByteArray();
    const char testArrayPrimitive[] = {
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
        0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
        0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
        0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
        0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7
    };
    auto byteArray = QByteArray::fromRawData(testArrayPrimitive, 64);
    if (byteArray.size() != PIXEL_TILE_TOTAL) {
        cerr << "Attempting to paint without " << PIXEL_TILE_TOTAL;
        cerr << " pixels! Found " << byteArray.size() << " pixels instead.";
        return;
    }

    for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
        char whichPaletteColor = byteArray.at(i);
        QColor qc("lightGray");
        if (whichPaletteColor == 0) {
            qc = QColor("red");
        } else if (whichPaletteColor == 1) {
            qc = QColor("blue");
        } else if (whichPaletteColor == 2) {
            qc = QColor("green");
        } else if (whichPaletteColor == 3) {
            qc = QColor("cyan");
        } else if (whichPaletteColor == 0xF) {
            qc = QColor("black");
        }
        // uint16_t colorBytes = (resultArray[writtenIndex+1] << 8) + resultArray[writtenIndex];
        // uint16_t red = colorBytes & 0b00000'00000'11111;
        // uint16_t green = (colorBytes & 0b00000'11111'00000) >> 5;
        // uint16_t blue = (colorBytes & 0b11111'00000'00000) >> 10;
        this->drawPixel(painter, option.rect, i % 8, i / 8, qc);
    }
    
}

void PixelDelegate::drawPixel(QPainter *painter, const QRect &rect, int x, int y, QColor &color) const {
    // Don't worry about memory waste, none of this is stored past function scope
    // Division gets weird with ints, use floats temporarily
    const float TRUE_HEIGHT = static_cast<float>(rect.height());
    const float TRUE_TOP = static_cast<float>(rect.y());
    const float SPLIT_HEIGHT = TRUE_HEIGHT / PIXEL_TILE_DIVISIONS;
    const float top = SPLIT_HEIGHT * y;
    const float TRUE_WIDTH = static_cast<float>(rect.width());
    const float TRUE_LEFT = static_cast<float>(rect.x());
    const float SPLIT_WIDTH = TRUE_WIDTH / PIXEL_TILE_DIVISIONS;
    const float left = SPLIT_WIDTH * x;
    painter->fillRect(
        std::round(TRUE_LEFT+left),
        std::round(TRUE_TOP+top),
        std::round(SPLIT_WIDTH),
        std::round(SPLIT_HEIGHT),
        color
    );
}


#endif