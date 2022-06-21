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
    QByteArray byteArray = index.data(PixelDelegateData::PIXEL_ARRAY).toByteArray();
    CollisionType colType = static_cast<CollisionType>(index.data(PixelDelegateData::COLLISIONTYPE).toInt());
    QByteArray palette = index.data(PixelDelegateData::PALETTE_ARRAY).toByteArray();
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
        char whichPalette = byteArray.at(i);
        auto qc = YUtils::getColorFromBytes(
            palette.at(whichPalette*2),
            palette.at(whichPalette*2+1)
        );
        int x = i % 8;
        if (index.data(PixelDelegateData::FLIP_H).toBool() == true) {
            x = (8 - x - 1);
        }
        int y = i / 8;
        if (index.data(PixelDelegateData::FLIP_V).toBool() == true) {
            y = (8 - y - 1);
        }
        this->drawPixel(painter, option.rect, x, y, qc);
    }
    if (index.data(PixelDelegateData::SHOW_COLLISION).toBool() == true) {
        const int squareMax = PIXEL_TILE_DIVISIONS-1;
        QColor qcBlack("black");
        qcBlack.setAlpha(150);
        QPen qpB(qcBlack);

        QColor qcWhite("white");
        qcWhite.setAlpha(200);
        QPen qpW(qcWhite);
        switch(colType) {
            case CollisionType::NONE:
                break;
            case CollisionType::SQUARE:
            default: {
                painter->setPen(qpB);
                painter->drawRect(option.rect.x(), option.rect.y(), squareMax, squareMax);
                
                painter->setPen(qpW);
                painter->drawRect(option.rect.x()+1,option.rect.y()+1,squareMax-2,squareMax-2);
                break;
            }
        }
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