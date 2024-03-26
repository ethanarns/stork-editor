#pragma once

#include "../yidsrom.h"
#include "BrushTable.h"

#include <string>

#include <QWidget>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>

class BrushWindow : public QWidget {
    Q_OBJECT
public:
    BrushWindow(QWidget *parent, YidsRom* rom);
    BrushTable* brushTable;
    QLineEdit* textboxBrushName;
    void stateChangedH(int state);
    void stateChangedV(int state);
    void loadSelectionClicked();
    void clearBrushClicked();
    bool saveCurrentBrushToFile();
    bool loadFileToCurrentBrush(std::string filename);

    QListWidget* stampList;
    void updateStampList();
    void saveBrushClicked();
    void stampListSelectedRowChanged(int currentRow);
    void deleteSelectedBrush();
    void loadBrushFile();
private:
    YidsRom* yidsRom;
};