#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <QtCore>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QAction>
#include <QLabel>

#include "yidsrom.h"
#include "DisplayTable.h"
#include "ChartilesTable.h"
#include "PaletteTable.h"
#include "SettingsEnum.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom(true);
    bool verbose = true;
    WindowEditMode windowEditMode = WindowEditMode::OBJECTS;

    MainWindow();
    void LoadRom();

    QLabel* paletteHoverLabel;
private:
    DisplayTable* grid;
    QWidget* chartilesPopup;
    ChartilesTable* chartilesTable;
    QWidget* palettePopup;
    PaletteTable* paletteTable;

    QAction* button_iconPalette;
    QAction* button_iconTiles;
    QAction* button_toggleCollision;

    void toolbarClick_palette();
    void toolbarClick_tiles();
    void toolbarClick_showCollision();
};

#endif