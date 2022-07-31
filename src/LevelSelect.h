#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include "yidsrom.h"

#include <QtCore>
#include <QListWidget>

class LevelSelect : public QListWidget {
    Q_OBJECT
public:
    LevelSelect(QWidget *parent, YidsRom* rom);
    void updateList();
    int wipeList();

    const static int ITEM_DATA_ID_CRSB = 101;
    const static int ITEM_DATA_ID_WORLD = 102;
    const static int ITEM_DATA_ID_LEVEL = 103;
private:
    YidsRom* yidsRom;

    const static int CELL_SIZE_HEIGHT_PX = 20;
};

#endif