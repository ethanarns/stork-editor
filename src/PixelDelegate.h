#ifndef PIXELDELEGATE_H
#define PIXELDELEGATE_H

#include <QtCore>
#include <QStyledItemDelegate>

enum PixelDelegateData {
    PIXEL_ARRAY = 101,
    PALETTE_ARRAY = 102,
    TILEATTR = 103,
    FLIP_H = 104,
    FLIP_V = 105,
    COLLISION_DRAW = 106,
    SHOW_COLLISION = 107
};

enum CollisionType {
    NONE = 0,
    SQUARE = 0x1,
    PLATFORM_PASSABLE = 0x2, // You can jump from below it and land on it, no head hit
    UP_RIGHT_45 = 0x7,
    UP_RIGHT_30 = 0x3,
    DOWN_RIGHT_45 = 0x47,
    DOWN_RIGHT_STEEP = 0x46,
    STATIC_COIN = 0x1a
};

enum CollisionDraw {
    CLEAR,
    CORNER_TOP_RIGHT,
    CORNER_TOP_LEFT,
    CORNER_BOTTOM_LEFT,
    CORNER_BOTTOM_RIGHT
};

class PixelDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
private:
    void drawPixel(QPainter *painter, const QRect &rect, int x, int y, QColor &color) const;
};
#endif