#ifndef PIXELDELEGATE_H
#define PIXELDELEGATE_H

#include <QtCore>
#include <QStyledItemDelegate>

class PixelDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};
#endif