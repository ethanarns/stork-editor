#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include "yidsrom.h"
#include "DisplayTable.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom(true);
    bool verbose = true;

    MainWindow();
    void LoadRom();
private:
    DisplayTable* grid;
};

#endif