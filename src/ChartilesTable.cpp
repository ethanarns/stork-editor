#include "ChartilesTable.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>

ChartilesTable::ChartilesTable(QWidget* parent) {
    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(ChartilesTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(ChartilesTable::CELL_SIZE_PX);
    this->setColumnCount(0x10);
    this->setRowCount(0x60);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");

    // Drag and Drop //
    this->setMouseTracking(true);
}