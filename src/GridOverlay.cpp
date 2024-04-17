#include "GridOverlay.h"
#include "GlobalSettings.h"
#include "yidsrom.h"
#include "data/MapData.h"

#include <iostream>

#include <QWidget>
#include <QFrame>
#include <QEvent>
#include <QPainter>

GridOverlay::GridOverlay(QWidget *viewport, YidsRom* yidsRom) : QFrame(viewport), rom(yidsRom) {
    if (this->rom == nullptr) {
        YUtils::printDebug("YidsRom was null for GridOverlay",DebugType::FATAL);
        YUtils::popupAlert("YidsRom was null for GridOverlay");
        exit(EXIT_FAILURE);
    }
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
        if (this->pathData != nullptr && this->needsPathUpdate) {
            this->handlePaths(&painter);
            this->needsPathUpdate = false;
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

bool GridOverlay::handlePaths(QPainter *paint) {
    if (this->pathData == nullptr) {
        return false;
    }
    for (auto mpit = this->pathData->paths.begin(); mpit != this->pathData->paths.end(); mpit++) {
        YUtils::printDebug("Handing parent path");
    }
    return true;
}
