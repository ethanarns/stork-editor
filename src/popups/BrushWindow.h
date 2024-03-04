#pragma once

#include "../yidsrom.h"
#include "BrushTable.h"

#include <QWidget>
#include <QCheckBox>

class BrushWindow : public QWidget {
    Q_OBJECT
public:
    BrushWindow(QWidget *parent, YidsRom* rom);
    BrushTable* brushTable;
    void stateChangedH(int state);
    void stateChangedV(int state);
    void loadSelectionClicked();
    void clearBrushClicked();
    bool saveCurrentBrushToFile();
private:
    YidsRom* yidsRom;
};