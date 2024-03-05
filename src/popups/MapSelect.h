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
    void cancelClicked();
    void confirmClicked();
private:
    QListWidget* leftList;
    QListWidget* rightList;
signals:
    void mpdzSelected(std::string mpdzNoExt);
};