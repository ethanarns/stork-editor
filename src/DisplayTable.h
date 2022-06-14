#ifndef DISPLAYTABLE_H
#define DISPLAYTABLE_H

#include <QtCore>
#include <QTableWidget>

class DisplayTable : public QTableWidget {
    Q_OBJECT
public:
    DisplayTable(QWidget *parent = Q_NULLPTR);
private:
    const static int CELL_SIZE_PX = 24;
};

#endif