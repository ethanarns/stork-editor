/**
 * @file MainWindow.cpp
 * @author @YoshiDonoshi (<email hidden>)
 * @brief This class is not only the base window, but also contains the connections to the ROM itself. It's the true main.
 * @version 0.1
 * @date 2022-06-12
 * 
 * @copyright Copyright (c) 2022
 */

#include "MainWindow.h"
#include "popups/ChartilesTable.h"
#include "popups/ObjTilesTable.h"
#include "popups/PaletteTable.h"
#include "utils.h"
#include "DisplayTable.h"
#include "GuiObjectList.h"
#include "data/LevelSelectData.h"
#include "GridOverlay.h"
#include "StateCommands.h"
#include "data/ProjectManager.h"

#include <QtCore>
#include <QObject>
#include <QWidget>
#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>
#include <QToolBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QStatusBar>
#include <QMessageBox>
#include <QSpinBox>
#include <QFrame>
#include <QCheckBox>

#include <iostream>
#include <filesystem>
#include <vector>

MainWindow::MainWindow() {
    YUtils::printDebug("** Launching Stork Editor **",DebugType::VERBOSE);

    // Set up settings
    globalSettings.currentBrush = new TileBrush();
    globalSettings.currentBrush->tileAttrs = std::vector<MapTileRecordData>();
    globalSettings.currentBrush->name = "errorName";
    globalSettings.currentBrush->brushTileset = "errorChar";
    globalSettings.currentBrush->brushWidth = 2;
    globalSettings.layerSelectMode = LayerMode::SPRITES_LAYER;

    if (!std::filesystem::exists("./lib/")) {
        std::stringstream ss;
        ss << "lib/ directory not found, was looking in '";
        ss << std::filesystem::current_path().string() << "'";
        YUtils::printDebug(ss.str(),DebugType::FATAL);
        QMessageBox::information( 
            this, 
            tr("Stork Editor"), 
            tr(ss.str().c_str()) );
        exit(EXIT_FAILURE);
    }
    this->setObjectName("mainWindow");
    QWidget* centralWidget = new QWidget;
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);
    
    /****************
     *** MENU BAR ***
     ****************/
    // File menu //
    QMenu* menu_file = menuBar()->addMenu("&File");

    QAction* action_open = new QAction("&Open",this);
    action_open->setShortcut(tr("CTRL+O"));
    action_open->setIcon(QIcon::fromTheme("document-open"));
    menu_file->addAction(action_open);
    // Always enabled, never disabled
    connect(action_open,&QAction::triggered,this,&MainWindow::LoadRom);

    QAction* action_close = new QAction("&Close File",this);
    // No shortcut
    action_close->setIcon(QIcon::fromTheme("window-close"));
    menu_file->addAction(action_close);
    action_close->setDisabled(true);
    //Add connect once implemeted

    menu_file->addSeparator();

    this->menu_save = new QAction("&Save",this);
    this->menu_save->setShortcut(tr("CTRL+S"));
    this->menu_save->setIcon(QIcon::fromTheme("document-save"));
    menu_file->addAction(this->menu_save);
    this->menu_save->setDisabled(true);
    connect(this->menu_save, &QAction::triggered, this, &MainWindow::saveRom);

    this->menu_export = new QAction("&Export...",this);
    menu_file->addAction(this->menu_export);
    this->menu_export->setDisabled(true);
    this->menu_export->setShortcut(tr("SHIFT+CTRL+S"));
    this->menu_export->setIcon(QIcon::fromTheme("document-save-as"));
    connect(this->menu_export, &QAction::triggered, this, &MainWindow::menuClick_export);

    menu_file->addSeparator();

    this->menu_levelSelect = new QAction("&Select Map...",this);
    this->menu_levelSelect->setShortcut(tr("CTRL+U"));
    //this->menu_levelSelect->setIcon(QIcon::fromTheme("document-page-setup"));
    menu_file->addAction(this->menu_levelSelect);
    this->menu_levelSelect->setDisabled(true);
    connect(this->menu_levelSelect, &QAction::triggered, this, &MainWindow::menuClick_levelSelect);

    menu_file->addSeparator();

    QAction* action_quit = new QAction("&Quit",this);
    action_quit->setShortcut(tr("CTRL+Q"));
    action_quit->setIcon(QIcon::fromTheme("application-exit"));
    menu_file->addAction(action_quit);
    // Always enabled, never disabled
    connect(action_quit,&QAction::triggered,qApp,QApplication::quit);

    // Edit menu //
    QMenu* menu_edit = menuBar()->addMenu("&Edit");

    this->action_undo = new QAction("&Undo",this);
    this->action_undo->setShortcut(tr("CTRL+Z"));
    this->action_undo->setIcon(QIcon::fromTheme("edit-undo"));
    menu_edit->addAction(this->action_undo);
    this->action_undo->setDisabled(true);
    connect(this->action_undo,&QAction::triggered,this,&MainWindow::undo);

    this->action_redo = new QAction("&Redo",this);
    this->action_redo->setShortcut(tr("CTRL+Y"));
    this->action_redo->setIcon(QIcon::fromTheme("edit-redo"));
    menu_edit->addAction(this->action_redo);
    this->action_redo->setDisabled(true);
    connect(this->action_redo,&QAction::triggered,this,&MainWindow::redo);

    menu_edit->addSeparator();

    QAction* action_cut = new QAction("&Cut",this);
    action_cut->setShortcut(tr("CTRL+X"));
    action_cut->setIcon(QIcon::fromTheme("edit-cut"));
    menu_edit->addAction(action_cut);
    action_cut->setDisabled(true);
    // Add connect() once implemented

    QAction* action_copy = new QAction("&Copy",this);
    action_copy->setShortcut(tr("CTRL+C"));
    action_copy->setIcon(QIcon::fromTheme("edit-copy"));
    menu_edit->addAction(action_copy);
    action_copy->setDisabled(true);
    // Add connect() once implemented

    QAction* action_paste = new QAction("&Paste",this);
    action_paste->setShortcut(tr("CTRL+V"));
    action_paste->setIcon(QIcon::fromTheme("edit-paste"));
    menu_edit->addAction(action_paste);
    action_paste->setDisabled(true);
    // Add connect() once implemented

    // View menu //
    QMenu* menu_view = menuBar()->addMenu("&View");

    this->action_viewBg1 = new QAction("&Show BG 1");
    this->action_viewBg1->setShortcut(tr("CTRL+1"));
    this->action_viewBg1->setCheckable(true);
    this->action_viewBg1->setChecked(true);
    this->action_viewBg1->setDisabled(true);
    menu_view->addAction(this->action_viewBg1);
    connect(this->action_viewBg1, &QAction::triggered, this, &MainWindow::menuClick_viewBg1);

    this->action_viewBg2 = new QAction("&Show BG 2");
    this->action_viewBg2->setShortcut(tr("CTRL+2"));
    this->action_viewBg2->setCheckable(true);
    this->action_viewBg2->setChecked(true);
    this->action_viewBg2->setDisabled(true);
    menu_view->addAction(this->action_viewBg2);
    connect(this->action_viewBg2, &QAction::triggered, this, &MainWindow::menuClick_viewBg2);

    this->action_viewBg3 = new QAction("&Show BG 3");
    this->action_viewBg3->setShortcut(tr("CTRL+3"));
    this->action_viewBg3->setCheckable(true);
    this->action_viewBg3->setChecked(true);
    this->action_viewBg3->setDisabled(true);
    menu_view->addAction(this->action_viewBg3);
    connect(this->action_viewBg3, &QAction::triggered, this, &MainWindow::menuClick_viewBg3);

    this->action_viewObjects = new QAction("&Show Sprites");
    this->action_viewObjects->setShortcut(tr("CTRL+4"));
    this->action_viewObjects->setCheckable(true);
    this->action_viewObjects->setChecked(true);
    this->action_viewObjects->setDisabled(true);
    menu_view->addAction(this->action_viewObjects);
    connect(this->action_viewObjects, &QAction::triggered, this, &MainWindow::menuClick_viewObjects);

    menu_view->addSeparator();

    this->action_showCollision = new QAction("&Show Collision");
    this->action_showCollision->setShortcut(tr("CTRL+5"));
    this->action_showCollision->setCheckable(true);
    this->action_showCollision->setChecked(true);
    this->action_showCollision->setDisabled(true);
    menu_view->addAction(this->action_showCollision);
    connect(this->action_showCollision, &QAction::triggered, this, &MainWindow::toolbarClick_showCollision);

    this->action_showTriggerBoxes = new QAction("&Show Triggers");
    this->action_showTriggerBoxes->setShortcut(tr("CTRL+6"));
    this->action_showTriggerBoxes->setCheckable(true);
    this->action_showTriggerBoxes->setChecked(true);
    this->action_showTriggerBoxes->setDisabled(true);
    menu_view->addAction(this->action_showTriggerBoxes);
    connect(this->action_showTriggerBoxes, &QAction::triggered,this, &MainWindow::menuClick_viewTriggers);

    this->action_showEntrances = new QAction("&Show Entrances");
    this->action_showEntrances->setShortcut(tr("CTRL+7"));
    this->action_showEntrances->setCheckable(true);
    this->action_showEntrances->setChecked(true);
    this->action_showEntrances->setDisabled(true);
    menu_view->addAction(this->action_showEntrances);
    connect(this->action_showEntrances, &QAction::triggered, this, &MainWindow::menuClick_viewEntrances);

    this->action_showExits = new QAction("&Show Exits");
    this->action_showExits->setShortcut(tr("CTRL+8"));
    this->action_showExits->setCheckable(true);
    this->action_showExits->setChecked(true);
    this->action_showExits->setDisabled(true);
    menu_view->addAction(this->action_showExits);
    connect(this->action_showExits, &QAction::triggered, this, &MainWindow::menuClick_viewExits);

    // Tools menu //
    QMenu* menu_tools = menuBar()->addMenu("&Tools");

    auto action_settings = new QAction("&Preferences...",this);
    action_settings->setShortcut(tr("CTRL+U"));
    action_settings->setIcon(QIcon::fromTheme("document-properties"));
    menu_tools->addAction(action_settings);
    action_settings->setDisabled(true); // Once implemented, never disable
    // Connect

    menu_tools->addSeparator();

    this->menu_levelSettings = new QAction("&Level Settings...",this);
    this->menu_levelSettings->setShortcut(tr("CTRL+L"));
    menu_tools->addAction(this->menu_levelSettings);
    this->menu_levelSettings->setDisabled(true);
    connect(this->menu_levelSettings, &QAction::triggered,this,&MainWindow::menuClick_levelSettings);

    QMenu* menu_help = menuBar()->addMenu("&Help");

    auto action_about = new QAction("&About",this);
    action_about->setShortcut(tr("F1"));
    action_about->setIcon(QIcon::fromTheme("help-about"));
    menu_help->addAction(action_about);
    connect(action_about,&QAction::triggered,this,&MainWindow::markSavableUpdate);
    // Add connect() once implemented
    
    /***************
     *** TOOLBAR ***
     ***************/

    QToolBar* toolbar = addToolBar("primary_toolbar");
    toolbar->setObjectName("toolbar_primary");
    toolbar->setMovable(false);

    QPixmap iconTiles(":/assets/icon_tiles.png");
    this->button_iconTiles = toolbar->addAction(QIcon(iconTiles), tr("Open BG Tiles Dialog"));
    this->button_iconTiles->setObjectName("button_iconTiles");
    this->button_iconTiles->setDisabled(true);
    connect(this->button_iconTiles, &QAction::triggered, this, &MainWindow::toolbarClick_tiles);

    QPixmap iconPalette(":/assets/icon_palette.png");
    this->button_iconPalette = toolbar->addAction(QIcon(iconPalette), tr("Open Palette Dialog"));
    this->button_iconPalette->setObjectName("button_iconPalette");
    this->button_iconPalette->setDisabled(true);
    connect(this->button_iconPalette, &QAction::triggered, this, &MainWindow::toolbarClick_palette);

    QPixmap iconSpritePreviewShow(":/assets/icon_spritepreview.png");
    this->button_showSpritePreview = toolbar->addAction(QIcon(iconSpritePreviewShow), tr("Open Sprite Preview"));
    this->button_showSpritePreview->setObjectName("button_spritePreview");
    this->button_showSpritePreview->setDisabled(true);
    connect(this->button_showSpritePreview,&QAction::triggered, this, &MainWindow::toolbarClick_spritePreview);

    QPixmap iconBrushWindowShow(":/assets/icon_brushwindow.png");
    this->button_iconBrush = toolbar->addAction(QIcon(iconBrushWindowShow), tr("Open BG Tile Brush window"));
    this->button_iconBrush->setObjectName("button_brushWindow");
    this->button_iconBrush->setDisabled(true);
    connect(this->button_iconBrush,&QAction::triggered, this,&MainWindow::toolbarClick_brush);

    // Collision Window //
    QPixmap iconColWindowShow(":/assets/collision_edit.png");
    this->button_colWindow = toolbar->addAction(QIcon(iconColWindowShow), tr("Open Collision Tile window"));
    this->button_colWindow->setObjectName("button_colWindow");
    this->button_colWindow->setDisabled(true);
    connect(this->button_colWindow,&QAction::triggered, this, &MainWindow::toolbarClick_col);

    QPixmap iconSpriteWindow(":/assets/coin.png");
    this->button_addSpriteWindow = toolbar->addAction(QIcon(iconSpriteWindow), tr("Open Sprite Picker"));
    this->button_addSpriteWindow->setObjectName("button_spritePickerWindow");
    this->button_addSpriteWindow->setDisabled(true);
    connect(this->button_addSpriteWindow,&QAction::triggered,this,&MainWindow::toolbarClick_spritePicker);

    toolbar->addSeparator();

    QPixmap iconCollisionShow(":/assets/icon_collision.png");
    this->button_toggleCollision = toolbar->addAction(QIcon(iconCollisionShow), tr("Toggle collision visibility"));
    this->button_toggleCollision->setObjectName("button_iconCollision");
    this->button_toggleCollision->setDisabled(true);
    connect(this->button_toggleCollision,&QAction::triggered, this, &MainWindow::toolbarClick_toggleCollision);

    toolbar->addSeparator();

    this->layerSelectDropdown = new QComboBox(this);
    this->layerSelectDropdown->setObjectName("layerSelectDropdown");
    this->layerSelectDropdown->setToolTip(tr("What type of items can be selected and moved"));
    this->layerSelectDropdown->addItem("Sprites");
    this->layerSelectDropdown->addItem("BG1");
    this->layerSelectDropdown->addItem("BG2");
    this->layerSelectDropdown->addItem("BG3");
    this->layerSelectDropdown->addItem("Colliders");
    this->layerSelectDropdown->addItem("Portals");
    this->layerSelectDropdown->setCurrentText("Sprites");
    this->layerSelectDropdown->setDisabled(true);
    toolbar->addWidget(this->layerSelectDropdown);
    connect(this->layerSelectDropdown,&QComboBox::currentTextChanged,this,&MainWindow::toolbarClick_layerSelect);

    toolbar->addSeparator();

    /**************
     *** LAYOUT ***
     **************/
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setObjectName(tr("layout_main"));

    // Left Panel //
    QVBoxLayout* leftPanelLayout = new QVBoxLayout(centralWidget);
    leftPanelLayout->setObjectName(tr("layout_left"));
    mainLayout->addLayout(leftPanelLayout);

    // Central Panel //
    QVBoxLayout* centerPanelLayout = new QVBoxLayout(centralWidget);
    centerPanelLayout->setObjectName(tr("layout_center"));
    mainLayout->addLayout(centerPanelLayout);

    // Initiate the layout //
    mainLayout->setStretch(0,2);
    mainLayout->setStretch(1,5);
    centralWidget->setLayout(mainLayout);

    /********************
     *** CENTER PANEL ***
     ********************/
    this->grid = new DisplayTable(this,this->rom);
    centerPanelLayout->addWidget(this->grid);
    connect(this->grid,&DisplayTable::clicked,this,&MainWindow::displayTableClicked);

    /******************
     *** LEFT PANEL ***
     ******************/
    qApp->setStyleSheet("QGroupBox { border: 1px solid gray;}"); // Debug

    // Top groupbox //
    QGroupBox* leftPanelTopGroupBox = new QGroupBox(tr("Tile Info"));
    leftPanelTopGroupBox->setObjectName("groupBox_leftPanel_top");
    leftPanelTopGroupBox->setStyleSheet("QGroupBox {padding-top: 22px;}");
    leftPanelLayout->addWidget(leftPanelTopGroupBox);
    QVBoxLayout* leftPanelTopGroupBoxLayout = new QVBoxLayout;

    this->selectionInfoTable = new SelectionInfoTable(this,this->rom,this->grid);
    leftPanelTopGroupBoxLayout->addWidget(this->selectionInfoTable);
    leftPanelTopGroupBox->setLayout(leftPanelTopGroupBoxLayout);
    leftPanelTopGroupBox->setMinimumHeight(300);

    // Bottom object list
    this->guiObjectList = new GuiObjectList(this,this->rom);
    this->guiObjectList->setObjectName("leftPanel_gui_object_list");
    leftPanelLayout->addWidget(this->guiObjectList);

    /*****************
     *** CHARTILES ***
     *****************/
    this->chartilesPopup = new QWidget; // No parent to avoid anchoring
    QHBoxLayout* chartilesLayout = new QHBoxLayout(this); // H cause it's skinny, so stuff next to each other
    this->chartilesTable = new ChartilesTable(this,this->rom); // Connect the rom
    chartilesLayout->addWidget(this->chartilesTable);
    // Side table //
    auto chartilesInfoLayout = new QVBoxLayout(this);

    auto chartilePaletteSelect = new QSpinBox(this);
    chartilePaletteSelect->setObjectName("chartilesTablePaletteSelect");
    chartilePaletteSelect->setEnabled(true);
    chartilePaletteSelect->setMinimum(0);
    chartilePaletteSelect->setMaximum(0xf);
    chartilePaletteSelect->setDisplayIntegerBase(16);
    chartilePaletteSelect->setPrefix(tr("0x"));
    chartilePaletteSelect->setToolTip(tr("Select color palette for the tiles here"));
    connect(chartilePaletteSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->chartilesTable,&ChartilesTable::paletteValueChanged);
    chartilesInfoLayout->addWidget(chartilePaletteSelect);

    chartilesLayout->addLayout(chartilesInfoLayout);

    this->chartilesPopup->setLayout(chartilesLayout);

    /*******************
     *** SPRITETILES ***
     *******************/
    this->objtilesPopup = new QWidget;
    QVBoxLayout* objtilesLayout = new QVBoxLayout(this);
    this->objtilesTable = new ObjTilesTable(this,this->rom);
    // Set up dropdown //
    this->spriteFileSelect = new QComboBox(this);
    this->spriteFileSelect->setObjectName("spriteFileSelectDropdown");
    this->spriteFileSelect->setToolTip(tr("Load an object render file"));
    this->spriteFileSelect->addItem("---");
    this->spriteFileSelect->addItem("objbutu.arcz");
    this->spriteFileSelect->addItem("objkantera.arcz");
    this->spriteFileSelect->addItem("objset.arcz");
    this->spriteFileSelect->addItem("objsbblock.arc");
    this->spriteFileSelect->addItem("objyamori.arcz");
    this->spriteFileSelect->addItem("objtori_yuuhi.arcz");
    this->spriteFileSelect->addItem("objplayer.arcz");
    this->spriteFileSelect->addItem("objgoal.arcz");

    this->spriteFileSelect->addItem("objadv_01a.arcz");
    this->spriteFileSelect->addItem("objadv_01b.arcz");
    this->spriteFileSelect->addItem("objmekakushikumo.arcz");
    this->spriteFileSelect->addItem("objop_01.arcz");
    this->spriteFileSelect->addItem("objtori_yuuhi.arcz");
    this->spriteFileSelect->addItem("objunbaba.arcz");
    this->spriteFileSelect->addItem("objgoallogo.arc");
    this->spriteFileSelect->addItem("objcrstitle.arc");

    this->spriteFileSelect->addItem("objhintwindow.arcz");
    this->spriteFileSelect->addItem("objeffect.arcz");
    this->spriteFileSelect->addItem("objtransform.arcz");

    //this->spriteFileSelect->addItem("title3_bg.arcz"); No OBAR found, crashes

    //this->spriteFileSelect->setCurrentText("---");
    this->spriteFileSelect->setEnabled(false);
    connect(this->spriteFileSelect,&QComboBox::currentTextChanged,this->objtilesTable,&ObjTilesTable::doFileLoad);
    objtilesLayout->addWidget(this->spriteFileSelect);

    // Selectors 1 //
    QHBoxLayout* spriteIndexesLayout = new QHBoxLayout(this);
    // OBJB select
    this->objbSelect = new QSpinBox(this);
    this->objbSelect->setObjectName("spritePopupObjbSelect");
    this->objbSelect->setEnabled(true);
    this->objbSelect->setMinimum(0);
    this->objbSelect->setMaximum(0xffff);
    this->objbSelect->setDisplayIntegerBase(16);
    this->objbSelect->setPrefix(tr("0x"));
    this->objbSelect->setToolTip(tr("Select which OBJB record to load"));
    // Frame select
    this->frameSelect = new QSpinBox(this);
    this->frameSelect->setObjectName("spritePopupFrameSelect");
    this->frameSelect->setEnabled(true);
    this->frameSelect->setMinimum(0);
    this->frameSelect->setDisplayIntegerBase(16);
    this->frameSelect->setPrefix(tr("0x"));
    this->frameSelect->setToolTip(tr("Select which frame of the object to render"));
    // Connect
    connect(this->objbSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->objtilesTable,&ObjTilesTable::objbValueChanged);
    connect(this->frameSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->objtilesTable,&ObjTilesTable::frameValueChanged);
    // Add selectors 1
    spriteIndexesLayout->addWidget(this->objbSelect);
    spriteIndexesLayout->addWidget(this->frameSelect);
    objtilesLayout->addLayout(spriteIndexesLayout);
    // Selectors 2 //
    QHBoxLayout* spriteWidthPaletteLayout = new QHBoxLayout(this);
    // Sprite width
    this->spriteWidthSelect = new QSpinBox(this);
    this->spriteWidthSelect->setObjectName("spritePopupWidthSelect");
    this->spriteWidthSelect->setEnabled(false);
    this->spriteWidthSelect->setMinimum(1);
    this->spriteWidthSelect->setDisplayIntegerBase(16);
    this->spriteWidthSelect->setPrefix(tr("W 0x"));
    this->spriteWidthSelect->setValue(2);
    this->spriteWidthSelect->setToolTip(tr("Set the width of the object to view (will be automatic later)"));
    // Sprite height
    this->spriteHeightSelect = new QSpinBox(this);
    this->spriteHeightSelect->setObjectName("spritePopupHeightSelect");
    this->spriteHeightSelect->setEnabled(false);
    this->spriteHeightSelect->setMinimum(1);
    this->spriteHeightSelect->setDisplayIntegerBase(16);
    this->spriteHeightSelect->setPrefix(tr("H 0x"));
    this->spriteHeightSelect->setValue(2);
    this->spriteHeightSelect->setToolTip(tr("Set the height of the object to view (will be automatic later)"));
    // Sprite palette
    this->spritePaletteSelect = new QSpinBox(this);
    this->spritePaletteSelect->setObjectName("spritePopupPaletteSelect");
    this->spritePaletteSelect->setEnabled(true);
    this->spritePaletteSelect->setMinimum(-1); // -1 = universal palette
    this->spritePaletteSelect->setMaximum(0xffff);
    this->spritePaletteSelect->setDisplayIntegerBase(16);
    this->spritePaletteSelect->setPrefix("0x");
    this->spritePaletteSelect->setValue(-1);
    this->spritePaletteSelect->setToolTip(tr("Select palette to use within the OBAR (-1 is universal palette)"));
    // Checkbox
    auto checkbox = new QCheckBox(this);
    checkbox->setText(tr("LZ10"));
    checkbox->setChecked(false);
    // Connect
    connect(this->spriteWidthSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->objtilesTable,&ObjTilesTable::widthChanged);
    connect(this->spriteHeightSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->objtilesTable,&ObjTilesTable::heightChanged);
    connect(this->spritePaletteSelect,QOverload<int>::of(&QSpinBox::valueChanged),this->objtilesTable,&ObjTilesTable::paletteChanged);
    connect(checkbox,&QCheckBox::stateChanged,this->objtilesTable,&ObjTilesTable::checkboxChanged);
    // Add selectors 2
    spriteWidthPaletteLayout->addWidget(this->spriteWidthSelect);
    spriteWidthPaletteLayout->addWidget(this->spriteHeightSelect);
    spriteWidthPaletteLayout->addWidget(this->spritePaletteSelect);
    spriteWidthPaletteLayout->addWidget(checkbox);
    objtilesLayout->addLayout(spriteWidthPaletteLayout);

    // Finishing touches //
    objtilesLayout->addWidget(this->objtilesTable);
    this->objtilesPopup->setLayout(objtilesLayout);

    /*********************
     *** PALETTE TABLE ***
     *********************/
    this->palettePopup = new QWidget;
    QVBoxLayout* paletteLayout = new QVBoxLayout(this); // Since it's wide, V, so stuff below and above
    this->paletteTable = new PaletteTable(this,this->rom);
    paletteLayout->addWidget(this->paletteTable);
    // Info area //
    QHBoxLayout* paletteInfoLayout = new QHBoxLayout(this);
    paletteLayout->addLayout(paletteInfoLayout);
    // Data
    auto label_colorShort = new QLabel;
    label_colorShort->setText(tr("Click for info"));
    label_colorShort->setObjectName("label_colorShort");
    paletteLayout->addWidget(label_colorShort);
    // Buttons
    QPushButton* nextPalettebutton = new QPushButton("&Next Palette",this);
    nextPalettebutton->setObjectName("button_nextPalette");
    paletteLayout->addWidget(nextPalettebutton);
    // Finalize //
    this->palettePopup->setLayout(paletteLayout);

    /********************
     *** LEVEL SELECT ***
     ********************/
    this->mapSelectPopup = new MapSelect(this,this->rom);
    connect(this->mapSelectPopup,&MapSelect::mpdzSelected,this,&MainWindow::mapPopupMpdzSelected);

    /******************
     *** STATUS BAR *** 
     ******************/
    this->statusBar()->show();
    this->statusBar()->setStyleSheet("QStatusBar {padding: 0; margin:0;}");
    this->statusLabel = new QLabel("Your Message");
    this->statusLabel->setStyleSheet("QLabel {padding:0;margin-top:0;margin-bottom:5px;margin-left:10px;}");
    this->statusBar()->addWidget(statusLabel);

    this->statusLabel->setText(tr("Ready"));

    // BrushWindow //
    this->brushWindow = new BrushWindow(this,this->rom);

    // ColWindow //
    this->colWindow = new ColWindow(this,this->rom);

    // SpritePickerWindow //
    this->spritePickerWindow = new SpritePickerWindow(this,this->rom);
    
    // Level data window //
    this->levelWindow = new LevelWindow(this,this->rom);

    // Project Manager //
    this->projectManager = new ProjectManager(this->rom);

    /*******************
     *** Connections ***
     *******************/
    connect(this->guiObjectList,&GuiObjectList::itemSelectionChanged,this,&MainWindow::objectListClick);
    connect(this->grid, &DisplayTable::triggerMainWindowUpdate,this,&MainWindow::displayTableUpdate);
    connect(this->selectionInfoTable, &SelectionInfoTable::pushCommandToUndoStack,this,&MainWindow::pushUndoableCommandToStack);
    connect(this->grid,&DisplayTable::updateMainWindowStatus,this,&MainWindow::setWindowStatus);
    connect(this->levelWindow,&LevelWindow::portalsUpdated,this,&MainWindow::portalsUpdated);
    connect(this->grid,&DisplayTable::pushStateCommandToStack,this,&MainWindow::pushUndoableCommandToStack);

    /***************
     *** OVERLAY ***
     ***************/
    this->gridOverlay = new GridOverlay(this->grid->viewport());
    this->gridOverlay->updateSizeToGrid(this->grid->rowCount(),this->grid->columnCount());
}

void MainWindow::LoadRom() {
    auto fileName = QFileDialog::getOpenFileName(this,tr("Open ROM"),".",tr("NDS files (*.nds *.NDS)"));
    if (fileName.isEmpty()) {
        YUtils::printDebug("Canceled file dialog",DebugType::VERBOSE);
    } else {
        this->statusLabel->setText(tr("Unpacking ROM..."));
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        YCompression::unpackRom(fileName.toStdString());
        this->currentFileName = ""; // Don't save to same rom file
        this->rom->openRom(fileName.toStdString());

        // Chartiles popup //
        this->chartilesPopup->resize(334, 400);
        this->chartilesPopup->setMinimumWidth(334);
        this->chartilesPopup->setWindowTitle("Select a layer to view its tileset");
        this->button_iconTiles->setDisabled(false);

        // Sprite tiles popup //
        this->objtilesPopup->resize(350,400);
        this->objtilesPopup->setMinimumWidth(350);
        this->objtilesPopup->setMinimumHeight(300);
        this->objtilesPopup->setWindowTitle("Select an object render data archive to view");
        this->spriteFileSelect->setEnabled(true);
        this->button_showSpritePreview->setDisabled(false);

        // Brush popup //
        this->button_iconBrush->setDisabled(false);

        // Col popup //
        this->button_colWindow->setDisabled(false);

        // Sprite Window //
        this->button_addSpriteWindow->setDisabled(false);
        this->spritePickerWindow->updateSpriteList("");

        // Level window //
        this->levelWindow->refreshLists();

        // Palette popup //
        this->palettePopup->resize(PaletteTable::PALETTE_TABLE_WINDOW_WIDTH,PaletteTable::PALETTE_TABLE_WINDOW_HEIGHT);
        this->palettePopup->setMinimumWidth(PaletteTable::PALETTE_TABLE_WINDOW_WIDTH);
        this->palettePopup->setMinimumHeight(PaletteTable::PALETTE_TABLE_WINDOW_HEIGHT);
        this->palettePopup->setWindowTitle("Palette Viewer");
        this->button_iconPalette->setDisabled(false);
        this->paletteTable->refreshLoadedTiles();

        // Level Select popup //
        this->menu_levelSelect->setDisabled(false);

        // Main table //
        this->grid->updateBg();
        this->grid->initCellCollision();
        this->button_toggleCollision->setDisabled(false);

        // Objects //
        this->grid->updateSprites();
        this->grid->setLayerDraw(4,true); // 4 here means objects
        this->layerSelectDropdown->setDisabled(false);
        this->toolbarClick_layerSelect("Sprites");

        // Triggers //
        this->grid->shouldShowTriggers = true;
        this->grid->updateTriggerBoxes();

        // Portals //
        this->grid->shouldDrawEntrances = true;
        this->grid->shouldDrawExits = true;
        this->grid->updatePortals(this->grid->shouldDrawEntrances,this->grid->shouldDrawExits);
        this->action_showEntrances->setDisabled(false);
        this->action_showExits->setDisabled(false);

        // Misc menu items //
        this->action_showCollision->setDisabled(false);
        this->action_viewBg1->setDisabled(false);
        this->action_viewBg2->setDisabled(false);
        this->action_viewBg3->setDisabled(false);
        this->action_viewObjects->setDisabled(false);
        this->action_showTriggerBoxes->setDisabled(false);
        this->grid->updateTriggerBoxes();
        this->menu_save->setDisabled(false);
        this->menu_export->setDisabled(false);
        this->menu_levelSettings->setDisabled(false);

        this->mapSelectPopup->updateLeftList();

        this->guiObjectList->updateList();
        this->statusLabel->setText(tr("ROM Loaded"));
    }
}

/**********************
 *** TOOLBAR CLICKS ***
 **********************/

void MainWindow::toolbarClick_palette() {
    if (this->palettePopup->isVisible()) {
        this->palettePopup->raise();
        this->palettePopup->activateWindow();
    } else {
        this->palettePopup->resize(550,660);
        this->palettePopup->show();
    }
}

void MainWindow::toolbarClick_tiles() {
    if (this->chartilesPopup->isVisible()) {
        this->chartilesPopup->raise();
        this->chartilesPopup->activateWindow();
    } else {
        this->chartilesPopup->show();
    }
}

void MainWindow::toolbarClick_spritePreview() {
    if (this->objtilesPopup->isVisible()) {
        this->objtilesPopup->raise();
        this->objtilesPopup->activateWindow();
    } else {
        this->objtilesPopup->show();
    }
}

void MainWindow::toolbarClick_brush() {
    if (this->brushWindow->isVisible()) {
        this->brushWindow->raise();
        this->brushWindow->activateWindow();
    } else {
        this->brushWindow->show();
    }
}

void MainWindow::toolbarClick_col() {
    if (this->colWindow->isVisible()) {
        this->colWindow->raise();
        this->colWindow->activateWindow();
    } else {
        this->colWindow->show();
    }
}

void MainWindow::toolbarClick_spritePicker() {
    if (this->spritePickerWindow->isVisible()) {
        this->spritePickerWindow->raise();
        this->spritePickerWindow->activateWindow();
    } else {
        this->spritePickerWindow->show();
    }
}

void MainWindow::toolbarClick_showCollision(bool shouldShow) {
    this->grid->shouldShowCollision = shouldShow;
    this->grid->updateShowCollision();
}

void MainWindow::toolbarClick_toggleCollision() {
    this->grid->shouldShowCollision = !this->grid->shouldShowCollision;
    this->action_showCollision->setChecked(this->grid->shouldShowCollision);
    this->grid->updateShowCollision();
}

void MainWindow::toolbarClick_layerSelect(const QString str) {
    auto strToCompare = str.toStdString();
    std::stringstream ss;
    this->grid->setDragEnabled(false);
    this->grid->setDragDropMode(QAbstractItemView::NoDragDrop);
    globalSettings.currentTileIndex = 0xffff; // Any change in layer resets the tile index
    this->brushWindow->brushTable->resetTable();
    this->brushWindow->curCharset->setText(tr(" "));
    ss << "Layer selected: ";
    if (str.compare("BG1") == 0) {
        globalSettings.layerSelectMode = LayerMode::BG1_LAYER;
        globalSettings.currentEditingBackground = 1;
        ss << "BG1";
        this->chartilesTable->refreshLoadedMapTilesMap(1);
        this->chartilesPopup->setWindowTitle("BG1 Tiles");
        this->brushWindow->updateStampList();
        this->brushWindow->updateCharsetLabel();
    } else if (str.compare("BG2") == 0) {
        globalSettings.layerSelectMode = LayerMode::BG2_LAYER;
        globalSettings.currentEditingBackground = 2;
        ss << "BG2";
        this->chartilesTable->refreshLoadedMapTilesMap(2);
        this->chartilesPopup->setWindowTitle("BG2 Tiles");
        this->brushWindow->updateStampList();
        this->brushWindow->updateCharsetLabel();
    } else if (str.compare("BG3") == 0) {
        globalSettings.layerSelectMode = LayerMode::BG3_LAYER;
        globalSettings.currentEditingBackground = 3;
        ss << "BG3";
        this->chartilesTable->refreshLoadedMapTilesMap(3);
        this->chartilesPopup->setWindowTitle("BG3 Tiles");
        this->brushWindow->updateStampList();
        this->brushWindow->updateCharsetLabel();
    } else if (str.compare("Sprites") == 0) {
        globalSettings.layerSelectMode = LayerMode::SPRITES_LAYER;
        globalSettings.currentEditingBackground = 0;
        this->grid->setDragEnabled(true);
        this->grid->setDragDropMode(QAbstractItemView::DragDrop);
        ss << "Sprites";
        // Do not use chartilesPopup
    } else if (str.compare("Colliders") == 0) {
        globalSettings.layerSelectMode = LayerMode::COLLISION_LAYER;
        globalSettings.currentEditingBackground = 0;
        ss << "Colliders";
    } else if (str.compare("Portals") == 0) {
        globalSettings.layerSelectMode = LayerMode::PORTALS_LAYER;
        globalSettings.currentEditingBackground = 0;
        ss << "Portals";
    } else {
        std::stringstream ssLayerSelect;
        ssLayerSelect << "Unknown layer selected in dropdown: ";
        ssLayerSelect << strToCompare << ", resetting to Sprites";
        YUtils::printDebug(ssLayerSelect.str(),DebugType::ERROR);
        globalSettings.currentEditingBackground = 0;
        globalSettings.layerSelectMode = LayerMode::SPRITES_LAYER;
        return;
    }
    YUtils::printDebug(ss.str(),DebugType::VERBOSE);
    this->grid->clearVisualSpriteSelection();
}

/*******************
 *** MENU CLICKS ***
 *******************/

void MainWindow::menuClick_levelSelect() {
    //std::cout << "Map Select Clicked" << std::endl;
    if (this->mapSelectPopup->isVisible()) {
        // It's still open, so just bring it to the front
        this->mapSelectPopup->activateWindow();
        this->mapSelectPopup->raise();
        return;
    }
    this->mapSelectPopup->show();
    //this->levelSelect->updateList();
}

void MainWindow::menuClick_export() {
    //this->saveRom();
    auto fileName = QFileDialog::getSaveFileName(this,tr("Export to NDS"),".",tr("NDS files (*.nds *.NDS)"));
    if (fileName.isEmpty()) {
        YUtils::printDebug("Canceled export dialog",DebugType::VERBOSE);
    } else {
        if (!fileName.endsWith(".nds")) {
            fileName = fileName.append(".nds");
        }
        std::stringstream ss;
        ss << "Exporting to " << fileName.toStdString() << "...";
        YUtils::printDebug(ss.str(),DebugType::VERBOSE);
        this->statusLabel->setText(tr("Exporting ROM..."));
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        YCompression::repackRom(fileName.toStdString());
        YUtils::printDebug("Export complete",DebugType::VERBOSE);
        this->statusLabel->setText(tr("ROM export complete"));
        this->setWindowTitle(Constants::WINDOW_TITLE);
    }
}

void MainWindow::menuClick_levelSettings() {
    //YUtils::printDebug("Opening level settings window",DebugType::VERBOSE);
    this->levelWindow->show();
    this->spritePickerWindow->raise();
    this->levelWindow->activateWindow();
}

void MainWindow::menuClick_viewExits(bool checked) {
    this->grid->updatePortals(this->grid->shouldDrawEntrances,checked);
}

void MainWindow::menuClick_viewEntrances(bool checked) {
    this->grid->updatePortals(checked,this->grid->shouldDrawExits);
}

/****************************
 *** WINDOW BUTTON CLICKS ***
 ****************************/

void MainWindow::mapPopupMpdzSelected(std::string mpdzNoExt) {
    std::stringstream ssMapPopup;
    ssMapPopup << "Loading MPDZ from popup: " << mpdzNoExt;
    YUtils::printDebug(ssMapPopup.str());
    this->grid->wipeTable();
    this->guiObjectList->wipeList();
    this->rom->wipeLevelData();
    this->rom->loadMpdz(mpdzNoExt);
    // Visual updates
    this->grid->firstLayerDrawDone = false;
    this->grid->updateBg();
    this->grid->initCellCollision();
    this->grid->updateSprites();
    this->grid->setLayerDraw(4,true);
    this->paletteTable->refreshLoadedTiles();
    this->levelWindow->refreshLists();
    this->guiObjectList->updateList();
    this->grid->updateTriggerBoxes();
    // If it was hidden last time, hide it again
    this->grid->updatePortals(this->grid->shouldDrawEntrances,this->grid->shouldDrawExits);
    this->brushWindow->updateCharsetLabel();
}

void MainWindow::menuClick_viewBg1(bool checked) {
    this->grid->setLayerDraw(1,checked);
}

void MainWindow::menuClick_viewBg2(bool checked) {
    this->grid->setLayerDraw(2,checked);
}

void MainWindow::menuClick_viewBg3(bool checked) {
    this->grid->setLayerDraw(3,checked);
}

void MainWindow::menuClick_viewObjects(bool checked) {
    this->grid->setLayerDraw(4,checked);
}

void MainWindow::menuClick_viewTriggers(bool checked) {
    this->grid->shouldShowTriggers = checked;
    this->grid->updateTriggerBoxes();
}

void MainWindow::saveRom() {
    this->statusLabel->setText(tr("Saving map file..."));
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    if (this->rom->mapData->filename.empty()) {
        // Saving a rom pre-load should be impossible
        YUtils::printDebug("No filename for MapData saved",DebugType::FATAL);
        YUtils::popupAlert("No filename for MapData saved");
        exit(EXIT_FAILURE);
    }
    this->setWindowTitle(Constants::WINDOW_TITLE);
    this->menu_save->setDisabled(true);
    ScenInfoData info; // Fake info
    auto outVec = this->rom->mapData->compile(info);
    auto finalOut = YCompression::lzssVectorRecomp(outVec);
    std::stringstream outFile;
    outFile << globalSettings.extractFolderName << "/data/file/" << this->rom->mapData->filename;
    std::stringstream ssSave;
    ssSave << "Saving map file '" << this->rom->mapData->filename << "' with size 0x" << std::hex << finalOut.size();
    ssSave << " / " << std::dec << finalOut.size();
    ssSave << " to location " << outFile.str();
    YUtils::printDebug(ssSave.str(),DebugType::VERBOSE);
    // Check if it exists already for debug purposes
    std::filesystem::path existing = outFile.str();
    if (std::filesystem::exists(existing)) {
        //YUtils::printDebug("Overwriting existing file",DebugType::VERBOSE);
    } else {
        YUtils::printDebug("No existing file found, creating new",DebugType::VERBOSE);
    }
    YUtils::writeByteVectorToFile(finalOut,outFile.str());

    // Now do the CRSB //
    auto compiledCrsb = this->rom->currentLevelSelectData->compile();
    auto crsbFilename = this->rom->currentLevelSelectData->filename;
    std::stringstream crsbOutFile;
    crsbOutFile << globalSettings.extractFolderName << "/data/file/" << crsbFilename;
    YUtils::writeByteVectorToFile(compiledCrsb,crsbOutFile.str());
    std::stringstream crsbResult;
    crsbResult << "Wrote CRSB to '" << crsbOutFile.str() << "'";
    YUtils::printDebug(crsbResult.str(),DebugType::VERBOSE);

    // Done!
    YUtils::printDebug("Map save successful");
    this->statusLabel->setText(tr("Map save successful"));
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::markSavableUpdate() {
    //YUtils::printDebug("Savable change made",DebugType::VERBOSE);
    this->menu_save->setDisabled(false);
    // Should already be enabled, but just in case
    std::string newWindowTitle = Constants::WINDOW_TITLE;
    newWindowTitle.append(" *");
    this->setWindowTitle(tr(newWindowTitle.c_str()));
}

void MainWindow::setWindowStatus(std::string status) {
    this->statusLabel->setText(tr(status.c_str()));
    // In times of heavy processing, it may not redraw, so it doesn't get shown
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
            // YUtils::printDebug("Delete selected objects",DebugType::VERBOSE);
            std::vector<uint32_t> selectedUuids = this->grid->selectedObjects;
            if (selectedUuids.empty()) {
                YUtils::printDebug("Nothing to delete");
                return;
            }
            for (auto it = selectedUuids.begin(); it != selectedUuids.end(); it++) {
                LevelObject spriteData = *this->rom->mapData->getLevelObjectByUuid(*it);
                DeleteSpriteCommand *del = new DeleteSpriteCommand(spriteData,this->grid,this->rom);
                this->undoStack->push(del);
                this->updateUndoMenu();
            }
            this->grid->selectedObjects.clear();
            this->grid->updateSprites();
            LevelObject empty;
            empty.objectId = 0xffff;
            empty.xPosition = 0;
            empty.yPosition = 0;
            empty.uuid = 0;
            empty.settingsLength = 0;
            empty.settings.clear();
            YUtils::printDebug("Updating selection info table with empty object");
            this->selectionInfoTable->updateWithLevelObject(&empty);
            this->guiObjectList->updateList();
            this->grid->clearVisualSpriteSelection();
            this->markSavableUpdate();
            return;
        } else {
            //YUtils::printDebug("Unhandled key in keyPressEvent for Sprites layer",DebugType::VERBOSE);
            return;
        }
    } else if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER
    ) {
        //YUtils::printDebug("keyPressEvent: BG Layer",DebugType::VERBOSE);
    } else if (globalSettings.layerSelectMode == LayerMode::COLLISION_LAYER) {
        //YUtils::printDebug("keyPressEvent: Collision layer",DebugType::VERBOSE);
    } else if (globalSettings.layerSelectMode == LayerMode::PORTALS_LAYER) {
        //YUtils::printDebug("keyPressEvent: Entrance/Exit layer",DebugType::VERBOSE);
    } else {
        MainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateUndoMenu() {
    std::stringstream ssUndo;
    ssUndo << "Undo " << this->undoStack->undoText().toStdString();
    this->action_undo->setText(QString::fromStdString(ssUndo.str()));
    std::stringstream ssRedo;
    ssRedo << "Redo " << this->undoStack->redoText().toStdString();
    this->action_redo->setText(QString::fromStdString(ssRedo.str()));

    if (this->undoStack->canUndo()) {
        this->action_undo->setDisabled(false);
    } else {
        this->action_undo->setDisabled(true);
    }
    
    if (this->undoStack->canRedo()) {
        this->action_redo->setDisabled(false);
    } else {
        this->action_redo->setDisabled(true);
    }
}

void MainWindow::objectListClick() {
    auto selectedItems = this->guiObjectList->selectedItems();
    if (selectedItems.size() == 0) {
        YUtils::printDebug("No items selected",DebugType::ERROR);
        return;
    }
    if (selectedItems.size() > 1) {
        YUtils::printDebug("Multiple items selection not supported",DebugType::WARNING);
        return;
    }
    QListWidgetItem* selectedItem = selectedItems.at(0);
    //auto objectId = (uint16_t)selectedItem->data(GuiObjectList::LEVEL_OBJECT_ID).toUInt();
    uint32_t objectUuid = (uint32_t)selectedItem->data(GuiObjectList::LEVEL_OBJECT_UUID).toUInt();
    // std::stringstream ss;
    // ss << "Selected object list object with UUID 0x" << std::hex << objectUuid;
    // YUtils::printDebug(ss.str(),DebugType::VERBOSE);
    auto loadedLevelObject = this->rom->mapData->getLevelObjectByUuid(objectUuid);
    if (loadedLevelObject == nullptr) {
        YUtils::printDebug("Loaded Level Object with that UUID not found",DebugType::ERROR);
        YUtils::popupAlert("Loaded Level Object with that UUID not found");
        return;
    }
    this->selectionInfoTable->updateWithLevelObject(loadedLevelObject);
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        this->selectionInfoTable->updateWithLevelObject(loadedLevelObject);
        auto itemTile = this->grid->item(loadedLevelObject->yPosition,loadedLevelObject->xPosition);
        if (itemTile == nullptr) {
            YUtils::printDebug("WidgetItem trying to scroll to is null",DebugType::ERROR);
            YUtils::popupAlert("WidgetItem trying to scroll to is null");
            return;
        }
        this->grid->scrollToItem(itemTile);
        this->grid->clearVisualSpriteSelection();
        this->grid->clearSelection();
        this->grid->selectedObjects.clear();
        this->grid->selectItemByUuid(objectUuid,false);
    }
}

void MainWindow::displayTableClicked() {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        auto selectedObjects = this->grid->selectedObjects;
        if (selectedObjects.size() < 1) {
            YUtils::printDebug("No objects selected in displayTableClicked->sprites",DebugType::VERBOSE);
            return;
        } else if (selectedObjects.size() > 1) {
            std::stringstream ss;
            ss << "More than one object selected (" << std::dec << selectedObjects.size() << "), taking first";
            YUtils::printDebug(ss.str(),DebugType::VERBOSE);
        }
        auto selectedObjectUuid = selectedObjects.at(0);
        // std::stringstream ssUuid;
        // ssUuid << "Attempting to find object with UUID 0x" << std::hex << selectedObjectUuid;
        // YUtils::printDebug(ssUuid.str(),DebugType::VERBOSE);
        LevelObject* lo = this->rom->mapData->getLevelObjectByUuid(selectedObjectUuid);
        if (lo == nullptr) {
            YUtils::printDebug("Invalid level object",DebugType::WARNING);
            return;
        }
        this->selectionInfoTable->updateWithLevelObject(lo);
    } else if (globalSettings.layerSelectMode == LayerMode::BG2_LAYER) {
        YUtils::printDebug("MainWindow clicked while in BG2 mode");
    } else if (globalSettings.layerSelectMode == LayerMode::BG1_LAYER) {
        YUtils::printDebug("MainWindow clicked while in BG1 mode");
    } else if (globalSettings.layerSelectMode == LayerMode::BG3_LAYER) {
        YUtils::printDebug("MainWindow clicked while in BG3 mode");
    } else if (globalSettings.layerSelectMode == LayerMode::COLLISION_LAYER) {
        YUtils::printDebug("MainWindow clicked while in COLL mode");
    } else if (globalSettings.layerSelectMode == LayerMode::PORTALS_LAYER) {
        YUtils::printDebug("MainWindow clicked while in PORTALS mode");
        YUtils::popupAlert("Portals mode selection not yet implemented");
    } else {
        YUtils::printDebug("MainWindow clicked while in UNKNOWN mode");
    }
}

void MainWindow::displayTableUpdate(){
    this->markSavableUpdate();
    auto selectedObjects = this->grid->selectedObjects;
    if (selectedObjects.size() < 1) {
        //YUtils::printDebug("No objects selected",DebugType::VERBOSE);
        return;
    } else if (selectedObjects.size() > 1) {
        std::stringstream ss;
        ss << "More than one object selected (" << std::dec << selectedObjects.size() << "), taking first";
        YUtils::printDebug(ss.str(),DebugType::VERBOSE);
    }
    auto selectedObjectUuid = selectedObjects.at(0);
    LevelObject* lo = this->rom->mapData->getLevelObjectByUuid(selectedObjectUuid);
    if (lo == nullptr) {
        YUtils::printDebug("Invalid level object",DebugType::ERROR);
        return;
    }
    this->selectionInfoTable->updateWithLevelObject(lo);
    this->guiObjectList->updateList();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    this->brushWindow->close();
    this->palettePopup->close();
    this->chartilesPopup->close();
    this->mapSelectPopup->close();
    this->objtilesPopup->close();
    this->colWindow->close();
    this->spritePickerWindow->close();
    this->levelWindow->close();
    event->accept();
}

void MainWindow::portalsUpdated() {
    this->grid->updatePortals(this->grid->shouldDrawEntrances,this->grid->shouldDrawExits);
    this->markSavableUpdate();
}

void MainWindow::undo() {
    this->undoStack->undo();
    this->updateUndoMenu();
    this->markSavableUpdate();
}

void MainWindow::redo() {
    this->undoStack->redo();
    this->updateUndoMenu();
    this->markSavableUpdate();
}

void MainWindow::pushUndoableCommandToStack(QUndoCommand *cmdPtr) {
    //YUtils::printDebug("pushUndoableCommandToStack");
    this->undoStack->push(cmdPtr);
    this->updateUndoMenu();
}
