#ifndef PIXELDELEGATE_H
#define PIXELDELEGATE_H

#include "PixelDelegateEnums.h"

#include <QtCore>
#include <QStyledItemDelegate>

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