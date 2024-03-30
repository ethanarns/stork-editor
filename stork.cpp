/**
 * @file stork.cpp
 * @author Zolarch (you@domain.com)
 * @brief The main file for Stork Editor
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <iostream>

#include <QApplication>

#include "src/MainWindow.h"
#include "src/constants.h"
#include "src/GlobalSettings.h"

GlobalSettings globalSettings;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // This class is the real main
    MainWindow window;

    if (argc == 2) {
        auto secondText = argv[1];
        QString second(secondText);
        if (second.compare("--version") == 0) {
            #ifdef APP_VERSION
            std::cout << APP_VERSION << std::endl;
            #else
            std::cout << "APP_VERSION not defined" << std::endl;
            #endif
            exit(EXIT_SUCCESS);
        }
    }

    window.resize(1200, 800);
    window.setMinimumWidth(800);
    window.setMinimumHeight(600);
    window.setWindowTitle(Constants::WINDOW_TITLE);
    window.show();

    return app.exec();
}
