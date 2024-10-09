#pragma once

#include "../yidsrom.h"
#include "../GridOverlay.h"
#include "qspinbox.h"

#include <QtCore>
#include <QWidget>
#include <QListWidget>

class PathWindow : public QWidget {
    Q_OBJECT
public:
    PathWindow(QWidget *parent, YidsRom* rom, GridOverlay* gOverlay);
    QListWidget* pathListWidget;
    QListWidget* pointListWidget;
    QSpinBox* xSpinBox;
    QSpinBox* ySpinBox;
    void refreshPathList();
    void refreshPointList();
    void pathListRowSelectionChanged(int rowIndex);
    void pointSelectionChanged(int rowIndex);
private:
    YidsRom* yidsRom;
    GridOverlay* gridOverlay;
    bool detectChanges = true;

    std::vector<PathSection *> getSelectedPathData();
    PathSection* getSelectedPathPoint();
    PathSection* getPathPoint(int index);

    void xSpinChange(int newValueX);
    void ySpinChange(int newValueY);
};
