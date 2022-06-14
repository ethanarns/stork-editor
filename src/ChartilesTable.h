#ifndef CHARTILESTABLE_H
#define CHARTILESTABLE_H

#include "yidsrom.h"

#include <QtCore>
#include <QTableWidget>

class ChartilesTable : public QTableWidget {
    Q_OBJECT
public:
    ChartilesTable(QWidget *parent, YidsRom* rom);
private:
    const static int CELL_SIZE_PX = 16;
    YidsRom* yidsRom;
};
#endif