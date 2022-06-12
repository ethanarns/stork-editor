#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include "yidsrom.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom(true);
    bool verbose = true;

    MainWindow();
    void LoadRom();
};

#endif