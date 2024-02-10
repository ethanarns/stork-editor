#include "SelectionInfoTable.h"

#include "yidsrom.h"

#include <QtCore>
#include <QWidget>
#include <QHeaderView>

SelectionInfoTable::SelectionInfoTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setColumnCount(2);
    this->setRowCount(4);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
}