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
#include "compression.h"
#include "GuiObjectList.h"
#include "data/LevelSelectData.h"

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

#include <iostream>
#include <filesystem>
#include <cstdio>

MainWindow::MainWindow() {
    YUtils::printDebug("** Launching Stork Editor **",DebugType::VERBOSE);

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

    // Nothing to save as, only export
    // this->menu_save_as = new QAction("&Save As...",this);
    // this->menu_save_as->setShortcut(tr("SHIFT+CTRL+S"));
    // this->menu_save_as->setIcon(QIcon::fromTheme("document-save-as"));
    // menu_file->addAction(this->menu_save_as);
    // this->menu_save_as->setDisabled(true);
    // connect(this->menu_save_as, &QAction::triggered, this, &MainWindow::saveRomAs);

    this->menu_export = new QAction("&Export...",this);
    menu_file->addAction(this->menu_export);
    this->menu_export->setDisabled(true);
    this->menu_export->setShortcut(tr("SHIFT+CTRL+S"));
    this->menu_export->setIcon(QIcon::fromTheme("document-save-as"));
    connect(this->menu_export, &QAction::triggered, this, &MainWindow::menuClick_export);

    menu_file->addSeparator();

    this->menu_levelSelect = new QAction("&Select Level...",this);
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

    QAction* action_undo = new QAction("&Undo",this);
    action_undo->setShortcut(tr("CTRL+Z"));
    action_undo->setIcon(QIcon::fromTheme("edit-undo"));
    menu_edit->addAction(action_undo);
    action_undo->setDisabled(true);
    // Add connect() once implemented

    QAction* action_redo = new QAction("&Redo",this);
    action_redo->setShortcut(tr("CTRL+Y"));
    action_redo->setIcon(QIcon::fromTheme("edit-redo"));
    menu_edit->addAction(action_redo);
    action_redo->setDisabled(true);
    // Add connect() once implemented

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

    this->action_viewObjects = new QAction("&Show Objects");
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

    // Tools menu //
    QMenu* menu_tools = menuBar()->addMenu("&Tools");

    auto action_settings = new QAction("&Settings...",this);
    action_settings->setShortcut(tr("CTRL+U"));
    action_settings->setIcon(QIcon::fromTheme("document-properties"));
    menu_tools->addAction(action_settings);
    action_settings->setDisabled(true); // Once implemented, never disable
    // Connect

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

    QPixmap iconTiles("assets/icon_tiles.png");
    this->button_iconTiles = toolbar->addAction(QIcon(iconTiles), tr("Open BG Tiles Dialog"));
    this->button_iconTiles->setObjectName("button_iconTiles");
    this->button_iconTiles->setDisabled(true);
    connect(this->button_iconTiles, &QAction::triggered, this, &MainWindow::toolbarClick_tiles);

    QPixmap iconPalette("assets/icon_palette.png");
    this->button_iconPalette = toolbar->addAction(QIcon(iconPalette), tr("Open Palette Dialog"));
    this->button_iconPalette->setObjectName("button_iconPalette");
    this->button_iconPalette->setDisabled(true);
    connect(this->button_iconPalette, &QAction::triggered, this, &MainWindow::toolbarClick_palette);

    toolbar->addSeparator();

    QPixmap iconCollisionShow("assets/icon_collision.png");
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
    this->layerSelectDropdown->setCurrentText("Sprites");
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

    this->selectionInfoTable = new SelectionInfoTable(this,this->rom);
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
    this->chartilesPopup->setLayout(chartilesLayout);

    /*******************
     *** SPRITETILES ***
     *******************/
    this->objtilesPopup = new QWidget;
    QVBoxLayout* objtilesLayout = new QVBoxLayout(this);
    this->objtilesTable = new ObjTilesTable(this,this->rom);
    // Set up dropdown //
    this->spriteFileSelectButton = new QPushButton("&Load file...", this);
    this->spriteFileSelectButton->setObjectName("spriteFileSelectButton");
    this->spriteFileSelectButton->setToolTip(tr("Load an object render file"));
    this->spriteFileSelectButton->setEnabled(false);
    connect(this->spriteFileSelectButton,&QPushButton::released,this->objtilesTable,&ObjTilesTable::doFileLoad);
    objtilesLayout->addWidget(this->spriteFileSelectButton);
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
    // Initial setup //
    this->levelSelectPopup = new QWidget;
    QVBoxLayout* levelSelectLayout = new QVBoxLayout(this);
    this->levelSelect = new LevelSelect(this,this->rom);
    levelSelectLayout->addWidget(this->levelSelect);
    this->levelSelectPopup->setLayout(levelSelectLayout);
    // Load and Cancel buttons //
    QHBoxLayout* levelSelectButtonsLayout = new QHBoxLayout(this);
    levelSelectLayout->addLayout(levelSelectButtonsLayout);
    // Load/Okay
    QPushButton* levelSelectButton_okay = new QPushButton("&Load",this);
    levelSelectButtonsLayout->addWidget(levelSelectButton_okay);
    connect(levelSelectButton_okay,&QPushButton::released, this, &MainWindow::buttonClick_levelSelect_load);
    // Cancel
    QPushButton* levelSelectButton_cancel = new QPushButton("&Cancel", this);
    levelSelectButtonsLayout->addWidget(levelSelectButton_cancel);
    connect(levelSelectButton_cancel,&QPushButton::released, this, &MainWindow::buttonClick_levelSelect_cancel);
    
    // Remaining qualities //
    this->levelSelectPopup->setWindowTitle("Select a level");

    this->statusBar()->show();
    this->statusBar()->setStyleSheet("QStatusBar {padding: 0; margin:0;}");
    this->statusLabel = new QLabel("Your Message");
    this->statusLabel->setStyleSheet("QLabel {padding:0;margin-top:0;margin-bottom:5px;margin-left:10px;}");
    this->statusBar()->addWidget(statusLabel);

    this->statusLabel->setText(tr("Ready"));

    /*******************
     *** Connections ***
     *******************/
    connect(this->guiObjectList,&GuiObjectList::itemSelectionChanged,this,&MainWindow::objectListClick);
    connect(this->grid, &DisplayTable::triggerMainWindowUpdate,this,&MainWindow::displayTableUpdate);
    connect(this->selectionInfoTable, &SelectionInfoTable::updateMainWindow,this,&MainWindow::selectionWindowUpdate);
    connect(this->grid,&DisplayTable::updateMainWindowStatus,this,&MainWindow::setWindowStatus);
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
        this->chartilesPopup->resize(350, 400);
        this->chartilesPopup->setMinimumWidth(350);
        this->chartilesPopup->setMinimumHeight(300);
        this->chartilesPopup->setWindowTitle("Select a layer to view its tileset");
        this->button_iconTiles->setDisabled(false);

        // Sprite tiles popup //
        this->objtilesPopup->resize(350,400);
        this->objtilesPopup->setMinimumWidth(350);
        this->objtilesPopup->setMinimumHeight(300);
        this->objtilesPopup->setWindowTitle("Select an object render data archive to view");
        this->spriteFileSelectButton->setEnabled(true);
        this->objtilesPopup->show();

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

        // Triggers //
        this->grid->updateTriggerBoxes();

        // Misc menu items //
        this->action_showCollision->setDisabled(false);
        this->action_viewBg1->setDisabled(false);
        this->action_viewBg2->setDisabled(false);
        this->action_viewBg3->setDisabled(false);
        this->action_viewObjects->setDisabled(false);
        this->menu_save->setDisabled(false);
        this->menu_export->setDisabled(false);

        this->guiObjectList->updateList();
        this->statusLabel->setText(tr("ROM Loaded"));
    }
}

/**********************
 *** TOOLBAR CLICKS ***
 **********************/

void MainWindow::toolbarClick_palette() {
    if (this->palettePopup->isVisible()) {
        this->palettePopup->close();
    } else {
        this->palettePopup->resize(550,660);
        this->palettePopup->show();
    }
}

void MainWindow::toolbarClick_tiles() {
    if (this->chartilesPopup->isVisible()) {
        this->chartilesPopup->close();
    } else {
        this->chartilesPopup->show();
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
    ss << "Layer selected: ";
    if (str.compare("BG1") == 0) {
        this->grid->layerSelectMode = LayerSelectMode::BG1_LAYER;
        ss << "BG1";
        this->chartilesTable->refreshLoadedMapTilesMap(1);
        this->chartilesPopup->setWindowTitle("BG1 Tiles");
    } else if (str.compare("BG2") == 0) {
        this->grid->layerSelectMode = LayerSelectMode::BG2_LAYER;
        ss << "BG2";
        this->chartilesTable->refreshLoadedMapTilesMap(2);
        this->chartilesPopup->setWindowTitle("BG2 Tiles");
    } else if (str.compare("BG3") == 0) {
        this->grid->layerSelectMode = LayerSelectMode::BG3_LAYER;
        ss << "BG3";
        this->chartilesTable->refreshLoadedMapTilesMap(3);
        this->chartilesPopup->setWindowTitle("BG3 Tiles");
    } else if (str.compare("Sprites") == 0) {
        this->grid->layerSelectMode = LayerSelectMode::SPRITES_LAYER;
        this->grid->setDragEnabled(true);
        this->grid->setDragDropMode(QAbstractItemView::DragDrop);
        ss << "Sprites";
        // Do this in a new window
        // this->chartilesTable->refreshLoadedObjectTilesMap();
        // this->chartilesPopup->setWindowTitle("Sprite Tiles");
    } else if (str.compare("Colliders") == 0) {
        this->grid->layerSelectMode = LayerSelectMode::COLLISION_LAYER;
        ss << "Colliders";
    } else {
        std::stringstream ssLayerSelect;
        ssLayerSelect << "Unknown layer selected in dropdown: ";
        ssLayerSelect << strToCompare;
        YUtils::printDebug(ssLayerSelect.str(),DebugType::ERROR);
        return;
    }
    YUtils::printDebug(ss.str(),DebugType::VERBOSE);
    this->grid->clearSelection();
}

/*******************
 *** MENU CLICKS ***
 *******************/

void MainWindow::menuClick_levelSelect() {
    //cout << "Level Select Clicked" << endl;
    if (this->levelSelectPopup->isVisible()) {
        // It's still open, so just bring it to the front
        this->levelSelectPopup->activateWindow();
        this->levelSelectPopup->raise();
        return;
    }
    this->levelSelectPopup->show();
    this->levelSelect->updateList();
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

/****************************
 *** WINDOW BUTTON CLICKS ***
 ****************************/

void MainWindow::buttonClick_levelSelect_load() {
    auto potentialCurrentItem = this->levelSelect->currentItem();
    if (potentialCurrentItem == nullptr) {
        return;
    }
    auto potentialItemCrsb = potentialCurrentItem->data(LevelSelect::ITEM_DATA_ID_CRSB);
    if (potentialCurrentItem == NULL || potentialCurrentItem == nullptr) {
        YUtils::printDebug("Invalid CRSB data attached to item",DebugType::ERROR);
        return;
    }
    this->grid->wipeTable();
    this->guiObjectList->wipeList();
    this->rom->wipeLevelData();
    auto loadingCrsb = potentialItemCrsb.toString().toStdString();
    std::stringstream ssLevelLoad;
    ssLevelLoad << "Loading CRSB (Level) '" << loadingCrsb << "'";
    YUtils::printDebug(ssLevelLoad.str(),DebugType::VERBOSE);

    auto fileNameCrsb_noext = YUtils::getLowercase(loadingCrsb);
    auto crsbFilename = fileNameCrsb_noext.append(".crsb");
    auto crsbFileVector = this->rom->getByteVectorFromFile(crsbFilename);
    auto crsbData = new LevelSelectData(crsbFileVector);

    auto firstMapName = crsbData->levels.at(0)->mpdzFileNoExtension;
    this->rom->loadMpdz(firstMapName);
    this->levelSelectPopup->close();
    // Visual updates
    this->grid->firstLayerDrawDone = false;
    this->grid->updateBg();
    this->grid->initCellCollision();
    this->grid->updateSprites();
    this->grid->setLayerDraw(4,true);
    //this->chartilesTable->refreshLoadedMapTilesMap(2);
    this->paletteTable->refreshLoadedTiles();
    this->guiObjectList->updateList();
    this->grid->updateTriggerBoxes();
}

void MainWindow::buttonClick_levelSelect_cancel() {
    this->levelSelectPopup->close();
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
    outFile << "_nds_unpack" << "/data/file/" << this->rom->mapData->filename;
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
    auto objectId = (uint16_t)selectedItem->data(GuiObjectList::LEVEL_OBJECT_ID).toUInt();
    auto textData = LevelObject::getObjectTextMetadata(objectId);
    uint32_t objectUuid = (uint32_t)selectedItem->data(GuiObjectList::LEVEL_OBJECT_UUID).toUInt();
    std::stringstream ss;
    ss << "Selected object list object with UUID 0x" << std::hex << objectUuid;
    YUtils::printDebug(ss.str(),DebugType::VERBOSE);
    auto loadedLevelObject = this->rom->mapData->getLevelObjectByUuid(objectUuid);
    if (loadedLevelObject == nullptr) {
        YUtils::printDebug("Loaded Level Object with that UUID not found",DebugType::ERROR);
        return;
    }
    this->selectionInfoTable->updateWithLevelObject(loadedLevelObject);
    if (this->grid->layerSelectMode == LayerSelectMode::SPRITES_LAYER) {
        this->grid->clearSelection();
        this->grid->selectItemByUuid(objectUuid);
    }
}

void MainWindow::displayTableClicked() {
    if (this->windowEditMode == WindowEditMode::OBJECTS) {
        auto selectedObjects = this->grid->selectedObjects;
        if (selectedObjects.size() < 1) {
            YUtils::printDebug("No objects selected",DebugType::VERBOSE);
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
    }
}

void MainWindow::displayTableUpdate(){
    if (this->windowEditMode == WindowEditMode::OBJECTS) {
        auto selectedObjects = this->grid->selectedObjects;
        if (selectedObjects.size() < 1) {
            YUtils::printDebug("No objects selected",DebugType::VERBOSE);
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
    this->markSavableUpdate();
}

void MainWindow::selectionWindowUpdate(LevelObject *sprite) {
    this->grid->wipeObject(sprite->uuid);
    this->grid->updateSprites();
    this->grid->clearSelection();
    this->grid->selectedObjects.clear();
    this->grid->selectItemByUuid(sprite->uuid);
}
