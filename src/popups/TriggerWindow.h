#pragma once

#include "../yidsrom.h"
#include "../DisplayTable.h"

#include <QWidget>
#include <QListWidget>
#include <QSpinBox>

class TriggerWindow : public QWidget {
    Q_OBJECT
public:
    TriggerWindow(QWidget *parent, YidsRom* rom, DisplayTable *grid);
    QListWidget* triggerList;

    void updateTriggerList();
private:
    YidsRom *yidsRom;
    DisplayTable *grid;

    QSpinBox *leftX;
    QSpinBox *topY;
    QSpinBox *rightX;
    QSpinBox *bottomY;

    void spinboxValueChanged(int i);
    void triggerListSelectionChanged(int currentRow);
    void addNewTrigger();
    void deleteTrigger();
    bool allowChanges = true;
signals:
    void markSavableChange();
};