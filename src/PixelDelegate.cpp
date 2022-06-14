#ifndef ROMSTRUCTS_H
#define ROMSTRUCTS_H

#include "PixelDelegate.h"

#include <QtWidgets>

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
    //QPen pen;
    auto frame = option.rect;
    painter->fillRect(frame.x(),frame.y(),2,2,QColor("black"));
}

#endif