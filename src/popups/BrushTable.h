#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QTableWidget>

class BrushTable : public QTableWidget {
    Q_OBJECT
public:
    BrushTable(QWidget *parent, YidsRom* rom);
    void resetTable();
    void loadTilesToCurBrush();
    void updateBrushDims();
    void setTile(int row, int column, MapTileRecordData tile);

    const static int CELL_COUNT_DIMS = 12;
protected:
    void mousePressEvent(QMouseEvent *event) override;
private:
    YidsRom* yidsRom;
    const static int CELL_SIZE_PX = 16;
};