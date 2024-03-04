#ifndef ROMSTRUCTS_H
#define ROMSTRUCTS_H

#include "PixelDelegate.h"
#include "PixelDelegateEnums.h"
#include "utils.h"

#include <iostream>
#include <sstream>

#include <QtCore>
#include <QtWidgets>
#include <QStaticText>
#include <QColor>
#include <QImage>

// A single drawn tile is 8x8
const int PIXEL_TILE_DIVISIONS = 8;
const int PIXEL_TILE_TOTAL = PIXEL_TILE_DIVISIONS * PIXEL_TILE_DIVISIONS;
//const QColor selectionColor(255,0,0,50);
const QColor hardSelectionColor(255,255,255,100);
QImage COIN_IMAGE(":/assets/coin.png");

const QColor collisionColor(     0,255,0  ,100);
const QColor collisionColorAlt(200,255,200,180);
const QColor collisionColorErr(255,0,0,180);

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
    using namespace std;
    const bool selected = (option.state & QStyle::State_Selected) != 0;

    QByteArray layerDrawOrder;
    if (index.data(PixelDelegateData::LAYER_DRAW_ORDER).isNull()) {
        const char drawOrderPrimitive[] = {1,2,3};
        layerDrawOrder = QByteArray::fromRawData(drawOrderPrimitive,3);
    } else {
        layerDrawOrder = index.data(PixelDelegateData::LAYER_DRAW_ORDER).toByteArray();
    }

    for (int layerOrderIndex = 0; layerOrderIndex < layerDrawOrder.size(); layerOrderIndex++) {
        auto whichBgToDraw = layerDrawOrder.at(layerOrderIndex);
        if (whichBgToDraw == 1) {
            /*****************
             *** BG1 TILES ***
            *****************/
            bool bg1selected = index.data(PixelDelegateData::TILE_SELECTED_BG1).toBool();
            if (index.data(PixelDelegateData::DRAW_BG1).toBool() == true) {
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
                auto paletteBg1 = index.data(PixelDelegateData::PALETTE_ARRAY_BG1).toByteArray();
                for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
                    QColor qc;
                    auto whichPalette = (uint8_t)byteArrayBg1.at(i);
                    if (whichPalette == 0 && index.data(PixelDelegateData::DRAW_TRANS_TILES).toBool() == false) {
                        // NOTE: Palette index 0 seems to almost always be the transparent
                        //   value. This may not be true, keep this in mind for future bugs
                        continue;
                    }
                    
                    auto firstByte = (uint8_t)paletteBg1[whichPalette*2];
                    auto secondByte = (uint8_t)paletteBg1[whichPalette*2+1];
                    qc = YUtils::getColorFromBytes(
                        firstByte,
                        secondByte
                    );
                    if (bg1selected) {
                        qc = YUtils::invertColor(qc);
                    }

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
            }
        } else if (whichBgToDraw == 2) {
            /*****************
             *** BG2 TILES ***
            *****************/
            bool bg2selected = index.data(PixelDelegateData::TILE_SELECTED_BG2).toBool();
            if (index.data(PixelDelegateData::DRAW_BG2).toBool() == true) {
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
                auto paletteBg2 = index.data(PixelDelegateData::PALETTE_ARRAY_BG2).toByteArray();
                for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
                    QColor qc;
                    auto whichPalette = (uint8_t)byteArrayBg2.at(i);
                    if (whichPalette == 0 && index.data(PixelDelegateData::DRAW_TRANS_TILES).toBool() == false) {
                        // NOTE: Palette index 0 seems to almost always be the transparent
                        //   value. This may not be true, keep this in mind for future bugs
                        continue;
                    }

                    auto firstByte = (uint8_t)paletteBg2[whichPalette*2];
                    auto secondByte = (uint8_t)paletteBg2[whichPalette*2+1];
                    qc = YUtils::getColorFromBytes(
                        firstByte,
                        secondByte
                    );
                    if (bg2selected) {
                        qc = YUtils::invertColor(qc);
                    }

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
            }
        } else if (whichBgToDraw == 3) {
            /*****************
             *** BG3 TILES ***
            *****************/
            bool bg3selected = index.data(PixelDelegateData::TILE_SELECTED_BG3).toBool();
            if (index.data(PixelDelegateData::DRAW_BG3).toBool() == true) {
                QByteArray byteArrayBg3 = index.data(PixelDelegateData::PIXEL_ARRAY_BG3).toByteArray();
                // Check the byte array size
                if (byteArrayBg3.size() == 0) {
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
                    byteArrayBg3 = QByteArray::fromRawData(testArrayPrimitive, 64);
                } else if (byteArrayBg3.size() != PIXEL_TILE_TOTAL) {
                    cerr << "Attempting to paint BG3 without " << PIXEL_TILE_TOTAL;
                    cerr << " pixels! Found " << byteArrayBg3.size() << " pixels instead." << endl;
                    return;
                }
                auto paletteBg3 = index.data(PixelDelegateData::PALETTE_ARRAY_BG3).toByteArray();
                for (int i = 0; i < PIXEL_TILE_TOTAL; i++) {
                    QColor qc;
                    auto whichPalette = (uint8_t)byteArrayBg3.at(i);
                    if (whichPalette == 0 && index.data(PixelDelegateData::DRAW_TRANS_TILES).toBool() == false) {
                        // NOTE: Palette index 0 seems to almost always be the transparent
                        //   value. This may not be true, keep this in mind for future bugs
                        continue;
                    }
                    
                    auto firstByte = (uint8_t)paletteBg3[whichPalette*2];
                    auto secondByte = (uint8_t)paletteBg3[whichPalette*2+1];
                    qc = YUtils::getColorFromBytes(
                        firstByte,
                        secondByte
                    );
                    if (bg3selected) {
                        qc = YUtils::invertColor(qc);
                    }

                    int x = i % 8;
                    if (index.data(PixelDelegateData::FLIP_H_BG3).toBool() == true) {
                        x = (8 - x - 1);
                    }
                    int y = i / 8;
                    if (index.data(PixelDelegateData::FLIP_V_BG3).toBool() == true) {
                        y = (8 - y - 1);
                    }
                    this->drawPixel(painter, option.rect, x, y, qc);
                }
            }
        } else if (whichBgToDraw == 0x0) {
            /* Do nothing */
        } else {
            std::stringstream ssBgOrderFail;
            ssBgOrderFail << "Unknown whichBgToDraw in PixelDelegate: 0x" << std::hex << (uint16_t)whichBgToDraw;
            YUtils::printDebug(ssBgOrderFail.str(),DebugType::WARNING);
            YUtils::printQbyte(layerDrawOrder);
            return;
        }
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

        const int X_WIDTH = option.rect.width();
        const int Y_HEIGHT = option.rect.height();
        const int X_BASE = option.rect.x();
        const int Y_BASE = option.rect.y();

        switch(colDrawType) {
            case CollisionDraw::CORNER_TOP_LEFT:
            case CollisionDraw::SQUARE_DRAW:
            case CollisionDraw::CORNER_TOP_RIGHT:
            case CollisionDraw::CORNER_BOTTOM_LEFT:
            case CollisionDraw::CORNER_BOTTOM_RIGHT: {
                QPainterPath path;
                path.addRect(X_BASE,Y_BASE,X_WIDTH,Y_HEIGHT);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::ZIG_ZAG: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH/2),(double)(Y_BASE+Y_HEIGHT));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColorAlt);
                break;
            }
            case CollisionDraw::DIAG_DOWN_RIGHT: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                poly << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::DIAG_UP_RIGHT: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE+Y_HEIGHT) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::DOWN_RIGHT_30_TALL: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT/2));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT)) << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::DOWN_RIGHT_30_SHORT: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE+Y_HEIGHT/2) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                poly << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::COIN_TOP_LEFT: {
                if (COIN_IMAGE.isNull()) {
                    COIN_IMAGE = QImage(":/assets/coin.png");
                }
                COIN_IMAGE = COIN_IMAGE.scaled(X_WIDTH*2,Y_HEIGHT*2); // Only need to do this once
                painter->drawImage(X_BASE,Y_BASE,COIN_IMAGE,0,0,X_WIDTH*2,Y_HEIGHT*2);
                break;
            }
            case CollisionDraw::COIN_BOTTOM_RIGHT: {
                painter->drawImage(X_BASE,Y_BASE,COIN_IMAGE,X_WIDTH,Y_HEIGHT,X_WIDTH*2,Y_HEIGHT*2);
                break;
            }
            case CollisionDraw::COIN_TOP_RIGHT: {
                painter->drawImage(X_BASE,Y_BASE,COIN_IMAGE,X_WIDTH,0,X_WIDTH*2,Y_HEIGHT*2);
                break;
            }
            case CollisionDraw::COIN_BOTTOM_LEFT: {
                painter->drawImage(X_BASE,Y_BASE,COIN_IMAGE,0,Y_HEIGHT,X_WIDTH*2,Y_HEIGHT*2);
                break;
            }
            case CollisionDraw::UP_RIGHT_30_BL: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE+Y_HEIGHT) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT/2));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::UP_RIGHT_30_BR: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE+Y_HEIGHT/2) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT)) << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::UPSIDE_DOWN_RIGHT_45: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break;
            }
            case CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_TALL: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT/2)) << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_SHORT: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT/2));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::UP_RIGHT_STEEP_SHORT: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE+X_WIDTH,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                poly << QPointF((double)(X_BASE+X_WIDTH/2),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::UP_RIGHT_STEEP_TALL: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE+Y_HEIGHT) << QPointF((double)(X_BASE+X_WIDTH/2),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE)) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::UPSIDE_DOWN_DOWNWARDS_45_DRAW: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::DOWN_RIGHT_STEEP_THIN: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH/2),(double)(Y_BASE+Y_HEIGHT));
                poly << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::DOWN_RIGHT_STEEP_WIDE: {
                QPainterPath path;
                QPolygonF poly;
                poly << QPointF((double)X_BASE,(double)Y_BASE) << QPointF((double)(X_BASE+X_WIDTH/2),(double)(Y_BASE));
                poly << QPointF((double)(X_BASE+X_WIDTH),(double)(Y_BASE+Y_HEIGHT)) << QPointF((double)(X_BASE),(double)(Y_BASE+Y_HEIGHT));
                path.addPolygon(poly);
                painter->fillPath(path,collisionColor);
                break; 
            }
            case CollisionDraw::SQERR: {
                QPainterPath path;
                path.addRect(X_BASE,Y_BASE,X_WIDTH,Y_HEIGHT);
                painter->fillPath(path,collisionColorErr);
                break;
            }
            case CollisionDraw::CLEAR:
            default:
                break;
        }
    }

    /***************
     *** OBJECTS ***
     ***************/
    if (index.data(PixelDelegateData::DRAW_OBJECTS).isNull() || index.data(PixelDelegateData::DRAW_OBJECTS).toBool() == true) {
        int objectId = index.data(PixelDelegateData::OBJECT_ID).toInt();
        Q_UNUSED(objectId);

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
                if (selected) {
                    this->drawPixel(painter, option.rect, x, y, hardSelectionColor);
                }
            }
        }

        if (!index.data(PixelDelegateData::OBJECT_ID).isNull() && index.data(PixelDelegateData::DRAW_OBJECTS).toBool()) {
            auto rectCopy = option.rect;
            rectCopy.setHeight(option.rect.height());
            rectCopy.setWidth(option.rect.width());
            painter->fillRect(rectCopy,"purple");
        }
    }
    
    /********************
     *** TRIGGERBOXES ***
     ********************/
    auto triggerData = index.data(PixelDelegateData::DRAW_TRIGGERS);
    if (!triggerData.toByteArray().isNull()) {
        auto triggerCount = triggerData.toByteArray().size();
        QColor boxColor(255, 0, 0, (30*triggerCount) );
        const int X_WIDTH = option.rect.width();
        const int Y_HEIGHT = option.rect.height();
        const int X_BASE = option.rect.x();
        const int Y_BASE = option.rect.y();
        painter->fillRect(
            X_BASE,
            Y_BASE,
            X_WIDTH,
            Y_HEIGHT,
            boxColor
        );
    }

    /*************
     *** HOVER ***
     *************/
    auto hoverType = index.data(PixelDelegateData::HOVER_TYPE);
    if (!hoverType.isNull() && hoverType.toInt() != HoverType::NO_HOVER) {
        HoverType hoverVal = static_cast<HoverType>(hoverType.toInt());
        QPen hoverPen;
        hoverPen.setColor(QColor("red"));
        hoverPen.setWidth(2);
        painter->setPen(hoverPen);
        switch (hoverVal) {
            case HoverType::HOVER_SQUARE: {
                auto rectCopy = option.rect;
                rectCopy.setX(rectCopy.x()+1);
                rectCopy.setY(rectCopy.y()+1);
                rectCopy.setHeight(option.rect.height()-2);
                rectCopy.setWidth(option.rect.width()-2);
                painter->drawRect(rectCopy);
                break;
            }
            case HoverType::HOVER_TOP: {
                painter->drawLine(option.rect.x(),option.rect.y()+1,option.rect.x()+option.rect.width()-1,option.rect.y()+1);
                break;
            }
            case HoverType::HOVER_LEFT: {
                painter->drawLine(option.rect.x()+1,option.rect.y(),option.rect.x()+1,option.rect.y()+option.rect.width()-1);
                break;
            }
            case HoverType::HOVER_BR: {
                // Bottom line
                painter->drawLine(
                    option.rect.x()+1,                     option.rect.y()+option.rect.width()-1,
                    option.rect.x()+option.rect.height()-1,option.rect.y()+option.rect.width()-1
                );
                // Right line
                painter->drawLine(
                    option.rect.x()+option.rect.height()-1,option.rect.y()+option.rect.width()-1,
                    option.rect.x()+option.rect.height()-1,option.rect.y()+1
                );
                break;
            }
            case HoverType::HOVER_TR: {
                // Top line
                painter->drawLine(
                    option.rect.x()+1,                    option.rect.y()+1,
                    option.rect.x()+option.rect.width()-1,option.rect.y()+1
                );
                // Right line
                painter->drawLine(
                    option.rect.x()+option.rect.height()-1,option.rect.y()+option.rect.width()-1,
                    option.rect.x()+option.rect.height()-1,option.rect.y()+1
                );
                break;
            }
            case HoverType::HOVER_BL: {
                // Bottom line
                painter->drawLine(
                    option.rect.x()+1,                    option.rect.y()+option.rect.height()-1,
                    option.rect.x()+option.rect.width()-1,option.rect.y()+option.rect.height()-1
                );
                // Left
                painter->drawLine(
                    option.rect.x()+1,option.rect.y()+1,
                    option.rect.x()+1,option.rect.y()+option.rect.width()-1
                );
                break;
            }
            case HoverType::HOVER_TL: {
                // Left
                painter->drawLine(
                    option.rect.x()+1,option.rect.y()+1,
                    option.rect.x()+1,option.rect.y()+option.rect.width()-1
                );
                // Top line
                painter->drawLine(
                    option.rect.x()+1,                    option.rect.y()+1,
                    option.rect.x()+option.rect.width()-1,option.rect.y()+1
                );
                break;
            }
            case HoverType::HOVER_RIGHT: {
                // Right line
                painter->drawLine(
                    option.rect.x()+option.rect.height()-1,option.rect.y()+option.rect.width()-1,
                    option.rect.x()+option.rect.height()-1,option.rect.y()+1
                );
                break;
            }
            case HoverType::HOVER_BOTTOM: {
                // Bottom line
                painter->drawLine(
                    option.rect.x()+1,     option.rect.y()+option.rect.width()-1,
                    option.rect.x()+option.rect.height()-1,option.rect.y()+option.rect.width()-1
                );
                break;
            }
            case HoverType::NO_HOVER: // This shouldn't be hit
            default: {
                break;
            }
        }
    }
}

void PixelDelegate::drawPixel(QPainter *painter, const QRect &rect, const int x, const int y, const QColor &color) const {
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