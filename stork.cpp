/**
 * @file stork.cpp
 * @author your name (you@domain.com)
 * @brief The main file for Stork Editor
 * @version 0.1
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <QApplication>

#include "src/MainWindow.h"
#include "src/constants.h"
#include "src/GlobalSettings.h"

GlobalSettings globalSettings;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // This class is the real main
    MainWindow window;
    window.resize(1200, 800);
    window.setMinimumWidth(800);
    window.setMinimumHeight(600);
    window.setWindowTitle(Constants::WINDOW_TITLE);
    window.show();

    return app.exec();
}