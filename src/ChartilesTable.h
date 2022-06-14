#ifndef CHARTILESTABLE_H
#define CHARTILESTABLE_H

#include <QtCore>
#include <QTableWidget>

class ChartilesTable : public QTableWidget {
    Q_OBJECT
public:
    ChartilesTable(QWidget *parent = Q_NULLPTR);
private:
    const static int CELL_SIZE_PX = 30;
};
#endif