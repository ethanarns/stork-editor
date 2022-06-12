#include <QtCore>
#include <QWidget>
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;

    window.resize(800, 600);
    window.setWindowTitle("Parasol Editor");
    window.show();

    return app.exec();
}