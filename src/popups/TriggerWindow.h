#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>
#include <QSpinBox>

class TriggerWindow : public QWidget {
    Q_OBJECT
public:
    TriggerWindow(QWidget *parent, YidsRom* rom);
    QListWidget* triggerList;

    void updateTriggerList();
private:
    YidsRom *yidsRom;

    QSpinBox *leftX;
    QSpinBox *topY;
    QSpinBox *rightX;
    QSpinBox *bottomY;

    void spinboxValueChanged(int i);
};