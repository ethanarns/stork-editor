#pragma once

#include "yidsrom.h"

#include <QFrame>
#include <QWidget>

class GridOverlay : public QFrame {
    Q_OBJECT
public:
    GridOverlay(QWidget* viewport, YidsRom* rom);
    void updateSizeToGrid(int rows, int columns);
    void drawPortals(QPainter &painter);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    YidsRom* yidsrom;
};