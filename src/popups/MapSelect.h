#pragma once

#include "../yidsrom.h"

#include <QWidget>
#include <QListWidget>

class MapSelect : public QWidget {
    Q_OBJECT
public:
    MapSelect(QWidget *parent, YidsRom* rom);
    YidsRom* yidsRom;
    void updateLeftList();
    void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void addMapClicked();
    void deleteMapClicked();
    void confirmClicked();
private:
    QListWidget* leftList;
    QListWidget* rightList;
    LevelSelectData* crsbTemp;
signals:
    void mpdzSelected(std::string mpdzNoExt);
};