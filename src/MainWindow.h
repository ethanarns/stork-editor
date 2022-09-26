#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <string>

#include <QtCore>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QAction>
#include <QLabel>
#include <QComboBox>

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

    void saveRom();
    void saveRomAs();
    void markSavableUpdate();

    QLabel* paletteHoverLabel;
    PaletteTable* paletteTable;

    std::string currentFileName = "";
private:
    DisplayTable* grid;
    QWidget* chartilesPopup;
    ChartilesTable* chartilesTable;
    QWidget* palettePopup;
    QWidget* levelSelectPopup;
    LevelSelect* levelSelect;
    QComboBox* layerSelectDropdown;

    QAction* button_iconPalette;
    QAction* button_iconTiles;
    QAction* button_toggleCollision;

    QAction* menu_levelSelect;
    QAction* menu_save;
    QAction* menu_save_as;
    QAction* action_memory;

    QAction* action_viewBg1;
    QAction* action_viewBg2;
    QAction* action_viewBg3;
    QAction* action_viewObjects;
    QAction* action_showCollision;

    void menuClick_viewBg1(bool checked);
    void menuClick_viewBg2(bool checked);
    void menuClick_viewBg3(bool checked);
    void menuClick_viewObjects(bool checked);

    void toolbarClick_palette();
    void toolbarClick_tiles();
    void toolbarClick_showCollision(bool shouldShow);
    void toolbarClick_toggleCollision();
    void toolbarClick_layerSelect(const QString str);

    void menuClick_levelSelect();
    void menuClick_memory();

    void buttonClick_levelSelect_cancel();
    void buttonClick_levelSelect_load();
};

#endif