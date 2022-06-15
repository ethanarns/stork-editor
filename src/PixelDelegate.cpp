#ifndef ROMSTRUCTS_H
#define ROMSTRUCTS_H

#include "PixelDelegate.h"
#include "utils.h"

#include <iostream>

#include <QtWidgets>

using namespace std;

// A single drawn tile is 8x8
const int PIXEL_TILE_DIVISIONS = 8;
const int PIXEL_TILE_TOTAL = PIXEL_TILE_DIVISIONS * PIXEL_TILE_DIVISIONS;

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
    //QPen pen;
    auto byteArray = index.data(PixelDelegateData::PIXEL_ARRAY).toByteArray();
    auto palette = index.data(PixelDelegateData::PALETTE_ARRAY).toByteArray();
    if (byteArray.size() == 0) {
        const char testArrayPrimitive[] = {
            0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
            0x1, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
            0x1, 0x1, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1,
            0x1, 0x1, 0x1, 0x0, 0x1, 0x1, 0x1, 0x1,
            0x1, 0x1, 0x1, 0x1, 0x0, 0x1, 0x1, 0x1,
            0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x1, 0x1,
            0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x1,
            0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0
        };
        byteArray = QByteArray::fromRawData(testArrayPrimitive, 64);
    } else if (byteArray.size() != PIXEL_TILE_TOTAL) {
        cerr << "Attempting to paint without " << PIXEL_TILE_TOTAL;
        cerr << " pixels! Found " << byteArray.size() << " pixels instead.";
        return;
    }
    
    for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
        char colIndex = byteArray.at(i);
        // QColor qc("lightGray");
        // if (colIndex == 0) {
        //     qc = QColor("red");
        // } else if (colIndex == 1) {
        //     qc = QColor("blue");
        // } else if (colIndex == 2) {
        //     qc = QColor("green");
        // } else if (colIndex == 3) {
        //     qc = QColor("cyan");
        // } else if (colIndex == 0xF) {
        //     qc = QColor("black");
        // }
        // uint16_t colorBytes = (resultArray[writtenIndex+1] << 8) + resultArray[writtenIndex];
        // uint16_t red = colorBytes & 0b00000'00000'11111;
        // uint16_t green = (colorBytes & 0b00000'11111'00000) >> 5;
        // uint16_t blue = (colorBytes & 0b11111'00000'00000) >> 10;
        auto qc = YUtils::getColorFromBytes(
            (colIndex*2),
            (colIndex*2)+1
        );
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