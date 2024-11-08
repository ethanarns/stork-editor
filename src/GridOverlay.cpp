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
    this->pathData = nullptr;
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
        if (this->pathData != nullptr && this->shouldRenderGridOverlay) {
            this->handlePaths(&painter);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

QPen magentaPen(QColor(245,66,149));
QPen pinkPen("pink");
QPen orangePen("orange");
QPen yellowPen("yellow");
constexpr int PATH_WIDTH_SMALL = 6;
constexpr int PATH_WIDTH_LARGE = 10;

bool GridOverlay::handlePaths(QPainter *paint) {
    int pathWidth = PATH_WIDTH_SMALL;
    int offset = 0;
    if (this->pathData == nullptr) {
        return false;
    }
    paint->setPen(pinkPen);
    int pathIndex = 0;
    int subPathIndex = 0;
    if (this->pathData->paths.empty()) {
        return false;
    }
    for (auto mpit = this->pathData->paths.cbegin(); mpit != this->pathData->paths.cend(); mpit++) {
        subPathIndex = 0;
        uint32_t priorFinalX = 0;
        uint32_t priorFinalY = 0;
        for (auto it = mpit->cbegin(); it != mpit->cend(); it++) {
            auto xFine = (*it)->xFine;
            auto yFine = (*it)->yFine;
            int finalX = (int)xFine >> 12;
            int finalY = (int)yFine >> 12;

            if (pathIndex == this->selectedPathIndex && subPathIndex == this->selectedPathSubIndex) {
                paint->setPen(magentaPen);
                pathWidth = PATH_WIDTH_LARGE;
                offset = pathWidth >> 1;
                paint->fillRect(finalX-offset,finalY-offset,pathWidth,pathWidth,pinkPen.color());
                paint->drawRect(finalX-offset,finalY-offset,pathWidth,pathWidth);
                paint->setPen(pinkPen);
                pathWidth = PATH_WIDTH_SMALL;
            } else {
                paint->setPen(pinkPen);
                offset = pathWidth >> 1;
                paint->fillRect(finalX-offset,finalY-offset,pathWidth,pathWidth,magentaPen.color());
                paint->drawRect(finalX-offset,finalY-offset,pathWidth,pathWidth);
            }

            if (subPathIndex != 0) {
                QLine subPathLine;
                //QLineF subPathLineF;
                if (pathIndex == this->selectedPathIndex) {
                    yellowPen.setWidth(2);
                    paint->setPen(yellowPen);
                }
                // subPathLineF.setP1(QPointF(priorFinalX,priorFinalY));
                // subPathLineF.setAngle(45);
                // subPathLineF.setLength((*it)->distance);
                // paint->drawLine(subPathLineF);
                subPathLine.setP1(QPoint(priorFinalX,priorFinalY));
                subPathLine.setP2(QPoint(finalX,finalY));
                paint->drawLine(subPathLine);
            }

            priorFinalX = finalX;
            priorFinalY = finalY;

            std::stringstream ssPath;
            ssPath << "" << pathIndex << "-" << subPathIndex;
            paint->setPen(orangePen);
            paint->drawText(finalX,finalY,QString::fromStdString(ssPath.str()));
            subPathIndex++;
        }
        pathIndex++;
    }
    return true;
}
