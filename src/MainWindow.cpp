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

#include <QtCore>
#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>

#include <iostream>
using namespace std;

MainWindow::MainWindow() {
    QWidget* centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    
    /****************
     *** MENU BAR ***
     ****************/
    // File menu
    QMenu* menu_file = menuBar()->addMenu("&File");

    QAction* action_open = new QAction("&Open",this);
    action_open->setShortcut(tr("CTRL+O"));
    menu_file->addAction(action_open);
    connect(action_open,&QAction::triggered,this,&MainWindow::LoadRom);

    menu_file->addSeparator();

    QAction* action_quit = new QAction("&Quit",this);
    action_quit->setShortcut(tr("CTRL+Q"));
    menu_file->addAction(action_quit);
    connect(action_quit,&QAction::triggered,qApp,QApplication::quit);

}

void MainWindow::LoadRom() {
    auto fileName = QFileDialog::getOpenFileName(this,tr("Open ROM"),".",tr("NDS files (*.NDS)"));
    if (fileName.isEmpty()) {
        cout << "Canceled file dialog" << endl;
    } else {
        this->rom->openRom(fileName.toStdString());
    }
}