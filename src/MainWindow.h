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
#include "popups/ChartilesTable.h"
#include "popups/PaletteTable.h"
#include "SettingsEnum.h"
#include "popups/LevelSelect.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom();
    WindowEditMode windowEditMode = WindowEditMode::OBJECTS;

    MainWindow();
    void LoadRom();

    QLabel* paletteHoverLabel;
    PaletteTable* paletteTable;
private:
    DisplayTable* grid;
    QWidget* chartilesPopup;
    ChartilesTable* chartilesTable;
    QWidget* palettePopup;
    QWidget* levelSelectPopup;
    LevelSelect* levelSelect;

    QAction* button_iconPalette;
    QAction* button_iconTiles;
    QAction* button_toggleCollision;

    QAction* menu_levelSelect;
    QAction* action_about;
    QAction* action_memory;

    QAction* action_viewBg1;
    QAction* action_viewBg2;
    QAction* action_viewBg3;

    void toolbarClick_palette();
    void toolbarClick_tiles();
    void toolbarClick_showCollision();

    void menuClick_levelSelect();
    void menuClick_memory();

    void buttonClick_levelSelect_cancel();
    void buttonClick_levelSelect_load();
};

#endif