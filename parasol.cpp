#include <QtCore>
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