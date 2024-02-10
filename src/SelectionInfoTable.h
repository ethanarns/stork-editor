#ifndef SELECTIONTABLE_H
#define SELECTIONTABLE_H

#include "yidsrom.h"
#include "LevelObject.h"

#include <QtCore>
#include <QTableWidget>

class SelectionInfoTable : public QTableWidget {
    Q_OBJECT
public:
    SelectionInfoTable(QWidget* parent, YidsRom* rom);
    void setText(int x, int y, std::string text, bool editable = false);
    void updateWithLevelObject(LevelObject lo);
private:
    YidsRom* yidsRom;
};

#endif