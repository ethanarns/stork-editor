#include "GridOverlay.h"
#include "GlobalSettings.h"

#include <iostream>

#include <QWidget>
#include <QFrame>
#include <QEvent>
#include <QPainter>

GridOverlay::GridOverlay(QWidget *viewport) : QFrame(viewport) {
    this->setObjectName("gridOverlay");
    this->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    this->setMouseTracking(false);
    this->installEventFilter(this);
    this->show();
}

void GridOverlay::updateSizeToGrid(int rows, int columns) {
    const int cellSize = globalSettings.gridCellSizePx;
    this->setFixedHeight(cellSize*rows);
    this->setFixedWidth(cellSize*columns);
}

bool GridOverlay::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Paint) {
        auto overlay = dynamic_cast<QWidget*>(obj);
        QPainter painter(overlay);

        return true;
    }
    return QObject::eventFilter(obj, event);
}
