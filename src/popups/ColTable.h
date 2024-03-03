#pragma once

#include <QTableWidget>

class ColTable : public QTableWidget {
    Q_OBJECT
public:
    ColTable(QWidget *parent);
    const static int CELL_PIXEL_DIMS = 32;
private:
};