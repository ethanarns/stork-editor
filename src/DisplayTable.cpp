#include "DisplayTable.h"
#include "PixelDelegate.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>

DisplayTable::DisplayTable(QWidget* parent) {
    Q_UNUSED(parent);

    // Layout //
    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(DisplayTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(DisplayTable::CELL_SIZE_PX);
    this->setRowCount(0xff);
    this->setColumnCount(0xff);
    //this->horizontalHeader()->hide();
    //this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");

    // Drag and Drop //
    this->setMouseTracking(true);

    setItemDelegate(new PixelDelegate);
}