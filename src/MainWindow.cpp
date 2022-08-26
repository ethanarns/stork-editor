/**
 * @file MainWindow.cpp
 * @author Ethan Messinger-Arns (<email hidden>)
 * @brief This class is not only the base window, but also contains the connections to the ROM itself. It's the true main.
 * @version 0.1
 * @date 2022-06-12
 * 
 * @copyright Copyright (c) 2022
 */

#include "MainWindow.h"
#include "popups/ChartilesTable.h"
#include "popups/PaletteTable.h"
#include "utils.h"
#include "DisplayTable.h"

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

#include <iostream>
using namespace std;

MainWindow::MainWindow() {
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
    menu_file->addAction(action_open);
    // Always enabled, never disabled
    connect(action_open,&QAction::triggered,this,&MainWindow::LoadRom);

    QAction* action_close = new QAction("&Close File",this);
    // No shortcut
    menu_file->addAction(action_close);
    action_close->setDisabled(true);
    //Add connect once implemeted

    menu_file->addSeparator();

    this->menu_levelSelect = new QAction("&Select Level...",this);
    this->menu_levelSelect->setShortcut(tr("CTRL+U"));
    menu_file->addAction(this->menu_levelSelect);
    this->menu_levelSelect->setDisabled(true);
    connect(this->menu_levelSelect, &QAction::triggered, this, &MainWindow::menuClick_levelSelect);

    menu_file->addSeparator();

    QAction* action_quit = new QAction("&Quit",this);
    action_quit->setShortcut(tr("CTRL+Q"));
    menu_file->addAction(action_quit);
    // Always enabled, never disabled
    connect(action_quit,&QAction::triggered,qApp,QApplication::quit);

    // Edit menu //
    QMenu* menu_edit = menuBar()->addMenu("&Edit");

    QAction* action_undo = new QAction("&Undo",this);
    action_undo->setShortcut(tr("CTRL+Z"));
    menu_edit->addAction(action_undo);
    action_undo->setDisabled(true);
    // Add connect() once implemented

    QAction* action_redo = new QAction("&Redo",this);
    action_redo->setShortcut(tr("CTRL+Y"));
    menu_edit->addAction(action_redo);
    action_redo->setDisabled(true);
    // Add connect() once implemented

    menu_edit->addSeparator();

    QAction* action_cut = new QAction("&Cut",this);
    action_cut->setShortcut(tr("CTRL+X"));
    menu_edit->addAction(action_cut);
    action_cut->setDisabled(true);
    // Add connect() once implemented

    QAction* action_copy = new QAction("&Copy",this);
    action_copy->setShortcut(tr("CTRL+C"));
    menu_edit->addAction(action_copy);
    action_copy->setDisabled(true);
    // Add connect() once implemented

    QAction* action_paste = new QAction("&Paste",this);
    action_paste->setShortcut(tr("CTRL+V"));
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

    this->action_memory = new QAction("&Memory Info",this);
    this->action_memory->setShortcut(tr("CTRL+M"));
    menu_tools->addAction(this->action_memory);
    this->action_memory->setDisabled(true);
    connect(this->action_memory, &QAction::triggered, this, &MainWindow::menuClick_memory);

    QMenu* menu_help = menuBar()->addMenu("&Help");

    this->action_about = new QAction("&About",this);
    //this->action_about->setShortcut(tr("CTRL+M"));
    menu_help->addAction(this->action_about);
    this->action_about->setDisabled(true);
    // Add connect() once implemented
    
    /***************
     *** TOOLBAR ***
     ***************/

    QToolBar* toolbar = addToolBar("primary_toolbar");
    toolbar->setObjectName("toolbar_primary");
    toolbar->setMovable(false);

    QPixmap iconTiles("assets/icon_tiles.png");
    this->button_iconTiles = toolbar->addAction(QIcon(iconTiles), tr("Open Tiles Dialog"));
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

    /******************
     *** LEFT PANEL ***
     ******************/
    qApp->setStyleSheet("QGroupBox {  border: 1px solid gray;}"); // Debug

    // Top groupbox //
    QGroupBox* leftPanelTopGroupBox = new QGroupBox(tr("Tile Info"));
    leftPanelTopGroupBox->setObjectName("groupBox_leftPanel_top");
    leftPanelLayout->addWidget(leftPanelTopGroupBox);
    QVBoxLayout* leftPanelTopGroupBoxLayout = new QVBoxLayout;

    this->paletteHoverLabel = new QLabel;
    this->paletteHoverLabel->setText(tr("Palette: 0xF"));
    leftPanelTopGroupBoxLayout->addWidget(this->paletteHoverLabel);
    leftPanelTopGroupBox->setLayout(leftPanelTopGroupBoxLayout);

    // Bottom groupbox //
    QGroupBox* leftPanelBottomGroupBox = new QGroupBox(tr("Objects"));
    leftPanelBottomGroupBox->setObjectName("groupBox_leftPanel_bottom");
    leftPanelLayout->addWidget(leftPanelBottomGroupBox);

    /*****************
     *** CHARTILES ***
     *****************/
    this->chartilesPopup = new QWidget; // No parent to avoid anchoring
    QHBoxLayout* chartilesLayout = new QHBoxLayout(this); // H cause it's skinny, so stuff next to each other
    this->chartilesTable = new ChartilesTable(this,this->rom); // Connect the rom
    chartilesLayout->addWidget(this->chartilesTable);
    this->chartilesPopup->setLayout(chartilesLayout);

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
    label_colorShort->setText(tr("Hover..."));
    label_colorShort->setObjectName("label_colorShort");
    paletteInfoLayout->addWidget(label_colorShort);
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
}

void MainWindow::LoadRom() {
    auto fileName = QFileDialog::getOpenFileName(this,tr("Open ROM"),".",tr("NDS files (*.NDS)"));
    if (fileName.isEmpty()) {
        YUtils::printDebug("Canceled file dialog",DebugType::VERBOSE);
    } else {
        this->rom->openRom(fileName.toStdString());

        // Chartiles popup //
        this->chartilesPopup->resize(300, 400);
        this->chartilesPopup->setMinimumWidth(300);
        this->chartilesPopup->setMinimumHeight(300);
        this->chartilesPopup->setWindowTitle("Tile Viewer");
        this->button_iconTiles->setDisabled(false);
        this->chartilesTable->refreshLoadedMapTilesMap(2);

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
        this->grid->updateObjects();

        // Misc menu items //
        this->action_memory->setDisabled(false);
        this->action_showCollision->setDisabled(false);
        this->action_viewBg1->setDisabled(false);
        this->action_viewBg2->setDisabled(false);
        this->action_viewBg3->setDisabled(false);
        this->action_viewObjects->setDisabled(false);
    }
}

/**********************
 *** TOOLBAR CLICKS ***
 **********************/

void MainWindow::toolbarClick_palette() {
    if (this->palettePopup->isVisible()) {
        this->palettePopup->close();
    } else {
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

void MainWindow::menuClick_memory() {
    std::cout << "----  Memory Report  ----" << std::endl;

    auto pixelTilesBg1size = this->rom->pixelTilesBg1.size();
    auto pixelTilesBg2size = this->rom->pixelTilesBg2.size();
    auto collisionArraySize = this->rom->collisionTileArray.size();
    auto preRenderDataBg1 = this->rom->preRenderDataBg1.size();
    auto preRenderDataBg2 = this->rom->preRenderDataBg2.size();
    auto levelObjectCount = this->rom->loadedLevelObjects.size();

    std::cout << "pixelTilesBg1 size (count): " << dec << pixelTilesBg1size << std::endl;
    std::cout << "pixelTilesBg2 size (count): " << dec << pixelTilesBg2size << std::endl;
    std::cout << "collisionTileArray size (bytes): " << dec << collisionArraySize << std::endl;
    std::cout << "preRenderDataBg1 size (count): " << dec << preRenderDataBg1 << std::endl;
    std::cout << "preRenderDataBg1 size (bytes): " << dec << preRenderDataBg1*2 << std::endl;
    std::cout << "preRenderDataBg2 size (count): " << dec << preRenderDataBg2 << std::endl;
    std::cout << "preRenderDataBg2 size (bytes): " << dec << preRenderDataBg2*2 << std::endl;
    std::cout << "loadedLevelObjects size (count): " << dec << levelObjectCount << std::endl;

    std::cout << "--- End Memory Report ---" << std::endl;
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
    this->rom->wipeCrsbData();
    auto loadingCrsb = potentialItemCrsb.toString().toStdString();
    std::stringstream ssLevelLoad;
    ssLevelLoad << "Loading CRSB (Level) '" << loadingCrsb << "'";
    YUtils::printDebug(ssLevelLoad.str(),DebugType::VERBOSE);
    auto crsb = this->rom->loadCrsb(loadingCrsb);
    this->rom->loadMpdz(crsb.cscnList.at(0).mpdzFileNoExtension);
    this->levelSelectPopup->close();
    // Visual updates
    this->grid->updateBg();
    this->grid->initCellCollision();
    this->grid->updateObjects();
    this->chartilesTable->wipeTiles();
    this->chartilesTable->refreshLoadedMapTilesMap(2);
    this->paletteTable->refreshLoadedTiles();
}

void MainWindow::buttonClick_levelSelect_cancel() {
    this->levelSelectPopup->close();
    this->rom->getGameLevelsMetaData();
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