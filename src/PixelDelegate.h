#ifndef PIXELDELEGATE_H
#define PIXELDELEGATE_H

#include <QtCore>
#include <QStyledItemDelegate>

enum PixelDelegateData {
    PIXEL_ARRAY_BG2 = 101,
    PALETTE_ARRAY_BG2 = 102,
    TILEATTR_BG2 = 103,
    FLIP_H_BG2 = 104,
    FLIP_V_BG2 = 105,

    COLLISION_DRAW = 106,
    SHOW_COLLISION = 107,
    DEBUG_DATA = 108,

    PIXEL_ARRAY_BG1 = 109,
    PALETTE_ARRAY_BG1 = 110,
    TILEATTR_BG1 = 111,
    FLIP_H_BG1 = 112,
    FLIP_V_BG1 = 113,

    /**
     * Type: int
     * Purpose: Object ID to print out
     */
    OBJECT_ID = 114,
    OBJECT_PALETTE = 115,
    OBJECT_TILES = 116,
    OBJECT_UUID = 117,

    DRAW_BG1 = 118,
    DRAW_BG2 = 119,
    DRAW_BG3 = 110,
    DRAW_OBJECTS = 111
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
    CORNER_BOTTOM_RIGHT,
    ZIG_ZAG,
    DIAG_DOWN_RIGHT,
    DIAG_UP_RIGHT
};

struct ObjectPalette {
    QByteArray paletteData;
    uint32_t index;
    uint32_t address;
};

class PixelDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
private:
    void drawPixel(QPainter *painter, const QRect &rect, const int x, const int y, const QColor &color) const;
};
#endif