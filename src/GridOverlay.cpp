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
        if (this->pathData != nullptr) {
            this->handlePaths(&painter);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

QColor magenta(245,66,149);
QColor white("white");
QColor yellow("yellow");

bool GridOverlay::handlePaths(QPainter *paint) {
    if (this->pathData == nullptr) {
        return false;
    }
    paint->setPen(white);
    int pathIndex = 0;
    int subPathIndex = 0;
    for (auto mpit = this->pathData->paths.cbegin(); mpit != this->pathData->paths.cend(); mpit++) {
        subPathIndex = 0;
        uint32_t priorFinalX = 0;
        uint32_t priorFinalY = 0;
        for (auto it = mpit->cbegin(); it != mpit->cend(); it++) {
            const int width = subPathIndex == 0 ? 10 : 5;
            auto xFine = (*it)->xFine;
            auto yFine = (*it)->yFine;
            int finalX = (int)xFine >> 12;
            int finalY = (int)yFine >> 12;

            if (subPathIndex != 0) {
                QLine subPathLine;
                if (pathIndex == this->selectedPathIndex) {
                    paint->setPen(yellow);
                }
                subPathLine.setP1(QPoint(priorFinalX,priorFinalY));
                subPathLine.setP2(QPoint(finalX,finalY));
                paint->drawLine(subPathLine);
            }

            priorFinalX = finalX;
            priorFinalY = finalY;

            paint->setPen(white);
            paint->fillRect(finalX,finalY,width,width,magenta);
            paint->drawRect(finalX,finalY,width,width);
            std::stringstream ssPath;
            ssPath << "" << pathIndex << "-" << subPathIndex;
            paint->drawText(finalX,finalY,QString::fromStdString(ssPath.str()));
            subPathIndex++;
        }
        pathIndex++;
    }
    return true;
}
