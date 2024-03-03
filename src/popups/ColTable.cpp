#include "ColTable.h"
#include "../PixelDelegate.h"
#include "../PixelDelegateEnums.h"
#include "../utils.h"

#include <iostream>

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>

ColTable::ColTable(QWidget *parent) {
    Q_UNUSED(parent);
    this->setColumnCount(2);
    this->setRowCount(1);

    this->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    this->horizontalHeader()->setMaximumSectionSize(ColTable::CELL_PIXEL_DIMS*1.5);
    this->setColumnWidth(0,ColTable::CELL_PIXEL_DIMS);
    this->horizontalHeader()->hide();

    this->verticalHeader()->setDefaultSectionSize(ColTable::CELL_PIXEL_DIMS);
    this->verticalHeader()->hide();

    this->setShowGrid(true);
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable text editing

    this->setItemDelegateForColumn(0,new PixelDelegate);

    for (int row = 0; row < this->rowCount(); row++) {
        auto curItem = this->item(row,0);
        if (curItem == nullptr) {
            curItem = new QTableWidgetItem();
            this->setItem(row,0,curItem);
        }
        curItem->setData(PixelDelegateData::SHOW_COLLISION,true);
        curItem->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::SQUARE_DRAW);
    }
}
