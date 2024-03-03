#pragma once

#include "../yidsrom.h"

#include <QWidget>

class ColWindow : public QWidget {
    Q_OBJECT
public:
    ColWindow(QWidget *parent, YidsRom* rom);
private:
    YidsRom *yidsRom;
};