#include "GridOverlay.h"
#include "GlobalSettings.h"

#include <iostream>

#include <QWidget>
#include <QFrame>
#include <QEvent>
#include <QPainter>

QImage EXIT_IMAGE(":/assets/exit.png");
QPen textPen("black");

GridOverlay::GridOverlay(QWidget *viewport) : QFrame(viewport) {
    this->setObjectName("gridOverlay");
    this->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    this->setMouseTracking(false);
    this->installEventFilter(this);
    this->show();
}

void GridOverlay::updateSizeToGrid(int rows, int columns) {
    //qf->setFixedHeight(DisplayTable::CELL_SIZE_PX*10);
    const int cellSize = globalSettings.gridCellSizePx;
    this->setFixedHeight(cellSize*rows);
    this->setFixedWidth(cellSize*columns);
}

void GridOverlay::drawExitAt(QPainter &painter, int row, int column) {
    if (EXIT_IMAGE.isNull()) {
        EXIT_IMAGE = QImage(":/assets/exit.png");
    }
    const uint scale = globalSettings.gridCellSizePx*2;
    EXIT_IMAGE = EXIT_IMAGE.scaled(scale,scale); // Only need to do this once
    painter.drawImage(
        row*globalSettings.gridCellSizePx,
        column*globalSettings.gridCellSizePx,
        EXIT_IMAGE,0,0,scale,scale);
    
    painter.setPen(textPen);
    QPoint base(
        row * globalSettings.gridCellSizePx+5,
        column * globalSettings.gridCellSizePx + scale-4);
    painter.drawText(base,tr("0"));
}

bool GridOverlay::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Paint) {
        auto overlay = dynamic_cast<QWidget*>(obj);
        QPainter painter(overlay);

        this->drawExitAt(painter,0x28,0x72);
        return true;
    }
    return QObject::eventFilter(obj, event);
}
