#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <QtCore>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QAction>

#include "yidsrom.h"
#include "DisplayTable.h"
#include "ChartilesTable.h"
#include "PaletteTable.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom(true);
    bool verbose = true;

    MainWindow();
    void LoadRom();
private:
    DisplayTable* grid;
    QWidget* chartilesPopup;
    ChartilesTable* chartilesTable;
    QWidget* palettePopup;
    PaletteTable* paletteTable;

    QAction* button_iconPalette;
    QAction* button_iconTiles;

    void toolbarClick_palette();
    void toolbarClick_tiles();
};

#endif