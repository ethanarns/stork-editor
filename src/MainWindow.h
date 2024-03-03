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
#include <QTextLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QCloseEvent>

#include "yidsrom.h"
#include "DisplayTable.h"
#include "popups/ChartilesTable.h"
#include "popups/PaletteTable.h"
#include "popups/ObjTilesTable.h"
#include "GlobalSettings.h"
#include "popups/LevelSelect.h"
#include "GuiObjectList.h"
#include "SelectionInfoTable.h"
#include "popups/BrushWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom();

    MainWindow();
    void LoadRom();

    void saveRom();
    void markSavableUpdate();

    SelectionInfoTable* selectionInfoTable;
    PaletteTable* paletteTable;
    QLabel* statusLabel;
    BrushWindow* brushWindow;

    std::string currentFileName = "";

    void setWindowStatus(std::string status);
private:
    DisplayTable* grid;
    QWidget* chartilesPopup;
    ChartilesTable* chartilesTable;

    QWidget* objtilesPopup;
    ObjTilesTable* objtilesTable;
    QComboBox* spriteFileSelect;
    QSpinBox* objbSelect;
    QSpinBox* frameSelect;
    QSpinBox* spriteWidthSelect;
    QSpinBox* spriteHeightSelect;
    QSpinBox* spritePaletteSelect;

    QWidget* palettePopup;
    QWidget* levelSelectPopup;
    LevelSelect* levelSelect;
    QComboBox* layerSelectDropdown;
    GuiObjectList* guiObjectList;

    QAction* button_iconPalette;
    QAction* button_iconTiles;
    QAction* button_showSpritePreview;
    QAction* button_iconBrush;
    QAction* button_toggleCollision;

    QAction* menu_levelSelect;
    QAction* menu_save;
    QAction* menu_export;

    QAction* action_viewBg1;
    QAction* action_viewBg2;
    QAction* action_viewBg3;
    QAction* action_viewObjects;
    QAction* action_viewTriggers;
    QAction* action_showCollision;
    QAction* action_showTriggerBoxes;

    void menuClick_viewBg1(bool checked);
    void menuClick_viewBg2(bool checked);
    void menuClick_viewBg3(bool checked);
    void menuClick_viewObjects(bool checked);
    void menuClick_viewTriggers(bool checked);

    void toolbarClick_palette();
    void toolbarClick_tiles();
    void toolbarClick_spritePreview();
    void toolbarClick_brush();
    void toolbarClick_showCollision(bool shouldShow);
    void toolbarClick_toggleCollision();
    void toolbarClick_layerSelect(const QString str);

    void menuClick_levelSelect();
    void menuClick_export();

    void buttonClick_levelSelect_cancel();
    void buttonClick_levelSelect_load();

    void objectListClick();
    void displayTableClicked();
    void displayTableUpdate();
    void selectionWindowUpdate(LevelObject *sprite);

    void closeEvent(QCloseEvent *event);
};

#endif