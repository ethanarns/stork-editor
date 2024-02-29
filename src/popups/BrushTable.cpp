#include "BrushTable.h"
#include "../PixelDelegate.h"

#include <QHeaderView>

BrushTable::BrushTable(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(BrushTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(BrushTable::CELL_SIZE_PX);
    this->setRowCount(BrushTable::CELL_COUNT_DIMS);
    this->setColumnCount(BrushTable::CELL_COUNT_DIMS);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setShowGrid(false);
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable text editing

    setItemDelegate(new PixelDelegate);
}