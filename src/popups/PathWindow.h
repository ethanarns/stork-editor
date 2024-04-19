#pragma once

#include "../yidsrom.h"
#include "../GridOverlay.h"

#include <QtCore>
#include <QWidget>
#include <QListWidget>

class PathWindow : public QWidget {
    Q_OBJECT
public:
    PathWindow(QWidget *parent, YidsRom* rom, GridOverlay* gOverlay);
    QListWidget* pathListWidget;
    QListWidget* subPathListWidget;
    void refreshPathList();
private:
    YidsRom* yidsRom;
    GridOverlay* gridOverlay;
    bool detectChanges = true;
};