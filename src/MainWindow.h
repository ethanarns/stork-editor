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
#include <QKeyEvent>
#include <QUndoStack>
#include <QUndoCommand>

#include "yidsrom.h"
#include "DisplayTable.h"
#include "popups/ChartilesTable.h"
#include "popups/PaletteTable.h"
#include "popups/ObjTilesTable.h"
#include "GlobalSettings.h"
#include "GuiObjectList.h"
#include "SelectionInfoTable.h"
#include "popups/BrushWindow.h"
#include "popups/ColWindow.h"
#include "popups/MapSelect.h"
#include "popups/SpritePickerWindow.h"
#include "popups/LevelWindow.h"
#include "GridOverlay.h"
#include "data/ProjectManager.h"
#include "popups/TriggerWindow.h"
#include "popups/PathWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    YidsRom* rom = new YidsRom();
    QUndoStack* undoStack = new QUndoStack();

    MainWindow();
    void LoadRom();

    void saveRom();
    void markSavableUpdate();

    SelectionInfoTable* selectionInfoTable;
    PaletteTable* paletteTable;
    QLabel* statusLabel;
    BrushWindow* brushWindow;
    ColWindow* colWindow;
    SpritePickerWindow* spritePickerWindow;
    TriggerWindow* triggerWindow;
    LevelWindow* levelWindow;
    ProjectManager* projectManager;
    PathWindow* pathWindow;

    std::string currentFileName = "";

    void setWindowStatus(std::string status);
    void updateUndoMenu();
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    DisplayTable* grid;
    GridOverlay* gridOverlay;
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
    MapSelect* mapSelectPopup;
    QComboBox* layerSelectDropdown;
    GuiObjectList* guiObjectList;

    QAction* button_iconPalette;
    QAction* button_iconTiles;
    QAction* button_showSpritePreview;
    QAction* button_iconBrush;
    QAction* button_colWindow;
    QAction* button_addSpriteWindow;
    QAction* button_triggerBoxWindow;
    QAction* button_pathsWindow;
    QAction* button_toggleCollision;

    QAction* menu_levelSelect;
    QAction* menu_save;
    QAction* menu_export;
    QAction* menu_levelSettings;
    
    QAction* action_undo;
    QAction* action_redo;
    QAction* action_copy;
    QAction* action_cut;
    QAction* action_paste;
    QAction* action_select_all;
    QAction* action_select_none;

    QAction* action_viewBg1;
    QAction* action_viewBg2;
    QAction* action_viewBg3;
    QAction* action_viewObjects;
    QAction* action_viewTriggers;
    QAction* action_showCollision;
    QAction* action_showTriggerBoxes;
    QAction* action_showEntrances;
    QAction* action_showExits;

    void menuClick_viewBg1(bool checked);
    void menuClick_viewBg2(bool checked);
    void menuClick_viewBg3(bool checked);
    void menuClick_viewObjects(bool checked);
    void menuClick_viewTriggers(bool checked);
    void menuClick_viewEntrances(bool checked);
    void menuClick_viewExits(bool checked);
    void menuClick_copy();
    void menuClick_cut();
    void menuClick_paste();
    void menuClick_selectAll();
    void menuClick_selectNone();

    void toolbarClick_palette();
    void toolbarClick_tiles();
    void toolbarClick_spritePreview();
    void toolbarClick_brush();
    void toolbarClick_col();
    void toolbarClick_spritePicker();
    void toolbarClick_triggerWindow();
    void toolbarClick_pathWindow();
    void toolbarClick_showCollision(bool shouldShow);
    void toolbarClick_toggleCollision();
    void toolbarClick_layerSelect(const QString str);

    void menuClick_levelSelect();
    void menuClick_export();
    void menuClick_levelSettings();
    void menuClick_about();

    void mapPopupMpdzSelected(std::string mpdzNameNoExt);

    void objectListClick();
    void displayTableClicked();
    void displayTableUpdate();
    void portalsUpdated();

    void closeEvent(QCloseEvent *event);
    void undo();
    void redo();
    void pushUndoableCommandToStack(QUndoCommand *cmdPtr);
    void updateClipboardUi();
    void updateOverlay();
};

#endif