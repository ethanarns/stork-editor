#pragma once

#include "yidsrom.h"

#include <QFrame>
#include <QWidget>
#include <QPainter>

class GridOverlay : public QFrame {
    Q_OBJECT
public:
    GridOverlay(QWidget* viewport, YidsRom* yidsRom);
    void updateSizeToGrid(int rows, int columns);
    PathData* pathData;
    int selectedPathIndex = -1;
    int selectedPathSubIndex = -1;
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    YidsRom *rom;
    bool handlePaths(QPainter *paint);
};