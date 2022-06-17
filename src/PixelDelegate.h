#ifndef PIXELDELEGATE_H
#define PIXELDELEGATE_H

#include <QtCore>
#include <QStyledItemDelegate>

enum PixelDelegateData {
    PIXEL_ARRAY = 101,
    PALETTE_ARRAY = 102,
    TILEATTR = 103,
    FLIP_H = 104,
    FLIP_V = 105
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