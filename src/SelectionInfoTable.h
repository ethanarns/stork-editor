#ifndef SELECTIONTABLE_H
#define SELECTIONTABLE_H

#include "yidsrom.h"

#include <QtCore>
#include <QTableWidget>

class SelectionInfoTable : public QTableWidget {
    Q_OBJECT
public:
    SelectionInfoTable(QWidget* parent, YidsRom* rom);
private:
    YidsRom* yidsRom;
};

#endif