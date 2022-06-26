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

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // This class is the real main
    MainWindow window;
    window.resize(800, 600);
    window.setMinimumWidth(600);
    window.setMinimumHeight(400);
    window.setWindowTitle("Parasol Editor");
    window.show();

    return app.exec();
}