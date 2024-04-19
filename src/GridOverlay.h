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
    int selectedPathIndex = 0;
    int selectedPathSubIndex = 5;
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    YidsRom *rom;
    bool handlePaths(QPainter *paint);
};