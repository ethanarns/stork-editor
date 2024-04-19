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
    // This is not a user setting, it prevents crashes during path data deletion
    bool shouldRenderGrid = true;
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    YidsRom *rom;
    bool handlePaths(QPainter *paint);
};