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
    CollisionDraw colDrawType = static_cast<CollisionDraw>(index.data(PixelDelegateData::COLLISION_DRAW).toInt());
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

        QColor qcBlack("black");
        QPen qpB(qcBlack);
        qpB.setStyle(Qt::PenStyle::SolidLine);

        QColor qcWhite("white");
        QPen qpW(qcWhite);
        qpW.setStyle(Qt::PenStyle::DotLine);

        const int X_WIDTH = option.rect.width()-1;
        const int Y_HEIGHT = option.rect.height()-1;
        const int X_BASE = option.rect.x();
        const int Y_BASE = option.rect.y();

        switch(colDrawType) {
            case CollisionDraw::CORNER_TOP_LEFT: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE,
                    Y_BASE+Y_HEIGHT
                );
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE,
                    Y_BASE+Y_HEIGHT
                );
                break;
            }
            case CollisionDraw::CORNER_TOP_RIGHT: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                painter->drawLine(
                    X_BASE+X_WIDTH,
                    Y_BASE,
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT
                );
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                painter->drawLine(
                    X_BASE+X_WIDTH,
                    Y_BASE,
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT
                );
                break;
            }
            case CollisionDraw::CORNER_BOTTOM_LEFT: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE,
                    Y_BASE+Y_HEIGHT
                );
                painter->drawLine(
                    X_BASE,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT
                );
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT
                );
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE,
                    Y_BASE+Y_HEIGHT
                );
                break;
            }
            case CollisionDraw::CORNER_BOTTOM_RIGHT: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT
                );
                painter->drawLine(
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT
                );
                painter->drawLine(
                    X_BASE+X_WIDTH,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                break;
            }
            case CollisionDraw::CLEAR:
            default:
                break;
            // case CollisionType::NONE:
            //     break;
            // case CollisionType::SQUARE:
            // default: {
            //     painter->setPen(qpB);
            //     painter->drawRect(option.rect.x(),option.rect.y(),squareMax,squareMax);
                
            //     painter->setPen(qpW);
            //     painter->drawRect(option.rect.x(),option.rect.y(),squareMax,squareMax);
            //     break;
            // }
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