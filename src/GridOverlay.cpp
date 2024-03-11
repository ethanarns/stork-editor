#include "GridOverlay.h"
#include "yidsrom.h"

#include <iostream>

#include <QWidget>
#include <QFrame>
#include <QEvent>
#include <QPainter>

GridOverlay::GridOverlay(QWidget *viewport, YidsRom* rom) : QFrame(viewport) {
    this->yidsrom = rom;
    this->setObjectName("gridOverlay");
    this->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    this->setMouseTracking(false);
    this->installEventFilter(this);
    this->show();
}

void GridOverlay::updateSizeToGrid(int rows, int columns) {
    //qf->setFixedHeight(DisplayTable::CELL_SIZE_PX*10);
    const int cellSize = 8;
    this->setFixedHeight(cellSize*rows);
    this->setFixedWidth(cellSize*columns);
}

void GridOverlay::drawPortals(QPainter &painter) {
    QPen pen("black");
    painter.setPen(pen);
    painter.fillRect(1,0x100,10,10,"green");
}

bool GridOverlay::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Paint) {
        auto overlay = dynamic_cast<QWidget*>(obj);
        QPainter painter(overlay);
        this->drawPortals(painter);
        return true;
    }
    return QObject::eventFilter(obj, event);
}
