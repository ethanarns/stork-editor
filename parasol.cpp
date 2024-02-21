/**
 * @file parasol.cpp
 * @author your name (you@domain.com)
 * @brief The main file for Parasol Editor
 * @version 0.1
 * @date 2022-06-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <QApplication>

#include <iostream>

#include "src/MainWindow.h"
#include "src/constants.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // This class is the real main
    MainWindow window;
    window.resize(1280, 1024);
    window.setMinimumWidth(800);
    window.setMinimumHeight(600);
    window.setWindowTitle(Constants::WINDOW_TITLE);
    window.show();

    return app.exec();
}