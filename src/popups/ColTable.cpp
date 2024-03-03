#include "ColTable.h"

#include "../PixelDelegate.h"
#include "../PixelDelegateEnums.h"
#include "../utils.h"
#include "../GlobalSettings.h"

#include <iostream>

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>

ColTable::ColTable(QWidget *parent) {
    Q_UNUSED(parent);
    this->setColumnCount(2);
    this->setRowCount(0xc7); // Highest known so far

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
    connect(this,&QTableWidget::cellClicked,this,&ColTable::tableCellClicked);

    for (int row = 0; row < 0xc7; row++) {
        this->updateRow(row,static_cast<CollisionType>(row));
    }
}

void ColTable::updateRow(int row, CollisionType colType) {
    if (row >= this->rowCount()) {
        this->setRowCount(row+1);
    }
    auto meta = YUtils::getCollisionMetadata(colType);
    // Update draw
    auto leftItem = this->item(row,0);
    if (leftItem == nullptr) {
        leftItem = new QTableWidgetItem();
        this->setItem(row,0,leftItem);
    }
    leftItem->setData(PixelDelegateData::SHOW_COLLISION,true);
    leftItem->setData(PixelDelegateData::COLLISION_DRAW,meta.preview);
    // Update text
    auto rightItem = this->item(row,1);
    if (rightItem == nullptr) {
        rightItem = new QTableWidgetItem();
        this->setItem(row,1,rightItem);
    }
    rightItem->setText(tr(meta.prettyName.c_str()));
}

void ColTable::tableCellClicked(int row, int column) {
    Q_UNUSED(column); // We don't care
    globalSettings.colTypeToDraw = static_cast<CollisionType>(row);
}
