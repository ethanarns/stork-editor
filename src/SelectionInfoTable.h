#ifndef SELECTIONTABLE_H
#define SELECTIONTABLE_H

#include "yidsrom.h"
#include "LevelObject.h"

#include <QtCore>
#include <QTableWidget>
#include <QTableWidgetItem>

class SelectionInfoTable : public QTableWidget {
    Q_OBJECT
public:
    SelectionInfoTable(QWidget* parent, YidsRom* rom);
    void setText(int x, int y, std::string text, bool editable = false);
    void updateWithLevelObject(LevelObject lo);
private:
    YidsRom* yidsRom;
    void cellDoubleClicked(int row, int column);
    void itemChanged(QTableWidgetItem *item);
    QTableWidgetItem* cellBeingEdited;
};

#endif