#include "GridOverlay.h"

#include <iostream>

#include <QWidget>
#include <QFrame>
#include <QEvent>
#include <QPainter>

GridOverlay::GridOverlay(QWidget *viewport) : QFrame(viewport) {
    std::cout << "GridOverlay" << std::endl;
    this->setObjectName("gridOverlay");
    this->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    this->setMouseTracking(false);
    this->installEventFilter(this);
}

void GridOverlay::updateSizeToGrid(int rows, int columns) {
    //qf->setFixedHeight(DisplayTable::CELL_SIZE_PX*10);
    const int cellSize = 8;
    this->setFixedHeight(cellSize*rows);
    this->setFixedWidth(cellSize*columns);
}

bool GridOverlay::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Paint) {
        auto overlay = dynamic_cast<QWidget*>(obj);
        QPainter painter(overlay);
        QPen pen("black");
        painter.setPen(pen);
        painter.fillRect(1,0x100,10,10,"green");
        return true;
    }
    return QObject::eventFilter(obj, event);
}
