#pragma once

#include "../yidsrom.h"
#include "BrushTable.h"

#include <string>

#include <QWidget>
#include <QCheckBox>
#include <QListWidget>

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
    bool loadFileToCurrentBrush(std::string filename);
    QListWidget* stampList;
private:
    YidsRom* yidsRom;
};