#pragma once

#include "../yidsrom.h"

#include <QtCore>
#include <QWidget>
#include <QListWidget>

class PathWindow : public QWidget {
    Q_OBJECT
public:
    PathWindow(QWidget *parent, YidsRom* rom);
    QListWidget* pathListWidget;
    QListWidget* subPathListWidget;
    void refreshLists();
private:
    YidsRom* yidsRom;
    bool detectChanges = true;
};