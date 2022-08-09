#ifndef ROMSTRUCTS_H
#define ROMSTRUCTS_H

#include "PixelDelegate.h"
#include "utils.h"

#include <iostream>

#include <QtCore>
#include <QtWidgets>
#include <QStaticText>

using namespace std;

// A single drawn tile is 8x8
const int PIXEL_TILE_DIVISIONS = 8;
const int PIXEL_TILE_TOTAL = PIXEL_TILE_DIVISIONS * PIXEL_TILE_DIVISIONS;

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {

    /*********************************
     *** PRIMARY (BG2) TILE PIXELS ***
     *********************************/
    QByteArray byteArrayBg2 = index.data(PixelDelegateData::PIXEL_ARRAY_BG2).toByteArray();
    // Check the byte array size
    if (byteArrayBg2.size() == 0) {
        const char testArrayPrimitive[] = {
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        };
        byteArrayBg2 = QByteArray::fromRawData(testArrayPrimitive, 64);
    } else if (byteArrayBg2.size() != PIXEL_TILE_TOTAL) {
        cerr << "Attempting to paint without " << PIXEL_TILE_TOTAL;
        cerr << " pixels! Found " << byteArrayBg2.size() << " pixels instead." << endl;
        return;
    }
    QByteArray paletteBg2 = index.data(PixelDelegateData::PALETTE_ARRAY_BG2).toByteArray();
    for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
        char whichPalette = byteArrayBg2.at(i);
        if (whichPalette == 0) {
            // NOTE: Palette index 0 seems to almost always be the transparent
            //   value. This may not be true, keep this in mind for future bugs
            continue;
        }
        auto qc = YUtils::getColorFromBytes(
            paletteBg2.at(whichPalette*2),
            paletteBg2.at(whichPalette*2+1)
        );
        int x = i % 8;
        if (index.data(PixelDelegateData::FLIP_H_BG2).toBool() == true) {
            x = (8 - x - 1);
        }
        int y = i / 8;
        if (index.data(PixelDelegateData::FLIP_V_BG2).toBool() == true) {
            y = (8 - y - 1);
        }
        this->drawPixel(painter, option.rect, x, y, qc);
    }

    /***********************************
     *** SECONDARY (BG1) TILE PIXELS ***
     ***********************************/
    QByteArray byteArrayBg1 = index.data(PixelDelegateData::PIXEL_ARRAY_BG1).toByteArray();
    // Check the byte array size
    if (byteArrayBg1.size() == 0) {
        const char testArrayPrimitive[] = {
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        };
        byteArrayBg1 = QByteArray::fromRawData(testArrayPrimitive, 64);
    } else if (byteArrayBg1.size() != PIXEL_TILE_TOTAL) {
        cerr << "Attempting to paint without " << PIXEL_TILE_TOTAL;
        cerr << " pixels! Found " << byteArrayBg1.size() << " pixels instead." << endl;
        return;
    }
    QByteArray paletteBg1 = index.data(PixelDelegateData::PALETTE_ARRAY_BG1).toByteArray();
    for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
        char whichPalette = byteArrayBg1.at(i);
        if (whichPalette == 0) {
            // NOTE: Palette index 0 seems to almost always be the transparent
            //   value. This may not be true, keep this in mind for future bugs
            continue;
        }
        auto qc = YUtils::getColorFromBytes(
            paletteBg1.at(whichPalette*2),
            paletteBg1.at(whichPalette*2+1)
        );
        int x = i % 8;
        if (index.data(PixelDelegateData::FLIP_H_BG1).toBool() == true) {
            x = (8 - x - 1);
        }
        int y = i / 8;
        if (index.data(PixelDelegateData::FLIP_V_BG1).toBool() == true) {
            y = (8 - y - 1);
        }
        this->drawPixel(painter, option.rect, x, y, qc);
    }

    /*****************
     *** COLLISION ***
     *****************/
    CollisionDraw colDrawType = static_cast<CollisionDraw>(index.data(PixelDelegateData::COLLISION_DRAW).toInt());
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
            case CollisionDraw::ZIG_ZAG: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE+X_WIDTH/2,
                    Y_BASE+Y_HEIGHT
                );
                painter->drawLine(
                    X_BASE+X_WIDTH/2,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                qpW.setColor("red");
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,
                    Y_BASE,
                    X_BASE+X_WIDTH/2,
                    Y_BASE+Y_HEIGHT
                );
                painter->drawLine(
                    X_BASE+X_WIDTH/2,
                    Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,
                    Y_BASE
                );
                break;
            }
            case CollisionDraw::DIAG_DOWN_RIGHT: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,Y_BASE,
                    X_BASE+X_WIDTH,Y_BASE+Y_HEIGHT
                );
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,Y_BASE,
                    X_BASE+X_WIDTH,Y_BASE+Y_HEIGHT
                );
                break;
            }
            case CollisionDraw::DIAG_UP_RIGHT: {
                painter->setPen(qpB);
                painter->drawLine(
                    X_BASE,Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,Y_BASE
                );
                painter->setPen(qpW);
                painter->drawLine(
                    X_BASE,Y_BASE+Y_HEIGHT,
                    X_BASE+X_WIDTH,Y_BASE
                );
            }
            case CollisionDraw::CLEAR:
            default:
                break;
        }
    }

    /***************
     *** OBJECTS ***
     ***************/
    int objectId = index.data(PixelDelegateData::OBJECT_ID).toInt();
    Q_UNUSED(objectId);
    if (!index.data(PixelDelegateData::OBJECT_ID).isNull()) {
        // QStaticText objectText("123");
        // QFont objectFont(tr(""),12,2,false);
        // painter->setFont(objectFont);
        // QPoint start = option.rect.topLeft();
        // start.setY(start.y()-(option.rect.height()/2));
        // painter->drawStaticText(start,objectText);
        QPen qcWhiteRect("white");
        painter->setPen(qcWhiteRect);
        auto rectCopy = option.rect;
        rectCopy.setHeight(option.rect.height()-1);
        rectCopy.setWidth(option.rect.width()-1);
        painter->fillRect(rectCopy,"purple");
        painter->drawRect(rectCopy);
    }

    auto objectTiles = index.data(PixelDelegateData::OBJECT_TILES).toByteArray();
    auto objectPalette = index.data(PixelDelegateData::OBJECT_PALETTE).toByteArray();
    if (!objectTiles.isNull() && !objectPalette.isNull()) {
        for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
            char whichPalette = objectTiles.at(i);
            if (whichPalette == 0) {
                // NOTE: Palette index 0 seems to almost always be the transparent
                //   value. This may not be true, keep this in mind for future bugs
                continue;
            }
            auto qc = YUtils::getColorFromBytes(
                objectPalette.at(whichPalette*2),
                objectPalette.at(whichPalette*2+1)
            );
            int x = i % 8;
            // if (index.data(PixelDelegateData::FLIP_H_BG1).toBool() == true) {
            //     x = (8 - x - 1);
            // }
            int y = i / 8;
            // if (index.data(PixelDelegateData::FLIP_V_BG1).toBool() == true) {
            //     y = (8 - y - 1);
            // }
            this->drawPixel(painter, option.rect, x, y, qc);
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