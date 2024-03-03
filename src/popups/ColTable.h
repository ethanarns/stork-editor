#pragma once

#include "../PixelDelegateEnums.h"

#include <QTableWidget>

class ColTable : public QTableWidget {
    Q_OBJECT
public:
    ColTable(QWidget *parent);
    const static int CELL_PIXEL_DIMS = 32;
private:
    void updateRow(int row, CollisionType colType);
};