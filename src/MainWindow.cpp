/**
 * @file MainWindow.cpp
 * @author Ethan Arns (<email hidden>)
 * @brief This class is not only the base window, but also contains the connections to the ROM itself. It's the true main.
 * @version 0.1
 * @date 2022-06-12
 * 
 * @copyright Copyright (c) 2022
 */

#include "MainWindow.h"
#include "ChartilesTable.h"
#include "PaletteTable.h"
#include "utils.h"

#include <QtCore>
#include <QWidget>
#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>
#include <QToolBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>

#include <iostream>
using namespace std;

MainWindow::MainWindow() {
    QWidget* centralWidget = new QWidget;
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
    
    /***************
     *** TOOLBAR ***
     ***************/

    QToolBar* toolbar = addToolBar("primary_toolbar");
    toolbar->setObjectName("toolbar_primary");
    toolbar->setMovable(false);
    QPixmap newpix("assets/icon_test.png");
    auto testAction = toolbar->addAction(QIcon(newpix), tr("Test"));
    Q_UNUSED(testAction);

    /**************
     *** LAYOUT ***
     **************/
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setObjectName(tr("layout_main"));

    // Left Panel //
    QVBoxLayout* leftPanelLayout = new QVBoxLayout(centralWidget);
    leftPanelLayout->setObjectName(tr("layout_left"));
    QPushButton* button1 = new QPushButton("Test1");
    leftPanelLayout->addWidget(button1);
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
    this->grid = new DisplayTable(centralWidget,this->rom);
    centerPanelLayout->addWidget(this->grid);

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
    this->palettePopup->setLayout(paletteLayout);
}

void MainWindow::LoadRom() {
    auto fileName = QFileDialog::getOpenFileName(this,tr("Open ROM"),".",tr("NDS files (*.NDS)"));
    if (fileName.isEmpty()) {
        cout << "Canceled file dialog" << endl;
    } else {
        this->rom->openRom(fileName.toStdString());

        // Chartiles popup //
        this->chartilesPopup->resize(300, 400);
        this->chartilesPopup->setMinimumWidth(300);
        this->chartilesPopup->setMinimumHeight(300);
        this->chartilesPopup->setWindowTitle("Tile Viewer");
        this->chartilesPopup->show();
        this->chartilesTable->refreshLoadedTiles();

        // Palette popup //
        this->palettePopup->resize(PaletteTable::PALETTE_TABLE_WINDOW_WIDTH,PaletteTable::PALETTE_TABLE_WINDOW_HEIGHT);
        this->palettePopup->setMinimumWidth(PaletteTable::PALETTE_TABLE_WINDOW_WIDTH);
        this->palettePopup->setMinimumHeight(PaletteTable::PALETTE_TABLE_WINDOW_HEIGHT);
        this->palettePopup->setWindowTitle("Palette Viewer");
        this->palettePopup->show();
        this->paletteTable->refreshLoadedTiles();

        // Main table //
        auto testPren = YUtils::getCharPreRender(0x7028);
        this->grid->putTile(0,0,testPren);
    }
}