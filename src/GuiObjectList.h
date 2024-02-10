#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include "yidsrom.h"

#include <QtCore>
#include <QListWidget>

class GuiObjectList : public QListWidget {
    Q_OBJECT
public:
    GuiObjectList(QWidget* parent, YidsRom* rom);
    void updateList();
    int wipeList();

    const static int LEVEL_OBJECT_UUID = 111;
private:
    YidsRom* yidsRom;
};

#endif