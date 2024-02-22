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
#include <sstream>

#include "src/MainWindow.h"
#include "src/constants.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    if (!std::filesystem::exists("./lib/")) {
        std::stringstream ss;
        ss << "lib/ directory not found, was looking in ";
        ss << std::filesystem::current_path().string();
        YUtils::printDebug(ss.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    // This class is the real main
    MainWindow window;
    window.resize(1280, 1024);
    window.setMinimumWidth(800);
    window.setMinimumHeight(600);
    window.setWindowTitle(Constants::WINDOW_TITLE);
    window.show();

    return app.exec();
}
