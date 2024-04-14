#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>

class TriggerWindow : public QWidget {
    Q_OBJECT
public:
    TriggerWindow(QWidget *parent, YidsRom* rom);
    QListWidget* triggerList;
private:
    YidsRom *yidsRom;
};