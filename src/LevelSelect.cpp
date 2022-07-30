#include "LevelSelect.h"
#include "yidsrom.h"

#include <QtCore>
#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>

#include <iostream>

LevelSelect::LevelSelect(QWidget *parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");

    this->updateList();
}

void LevelSelect::updateList() {
    QListWidgetItem* item = new QListWidgetItem(tr("Item 1"));
    item->setData(LevelSelect::ITEM_DATA_ID,69);
    std::cout << item->data(LevelSelect::ITEM_DATA_ID).toInt() << std::endl;
    this->addItem(item);

    auto len = this->count();
    for (int i = 0; i < len; i++) {
        auto curItem = this->item(i);
        std::cout << curItem->text().toStdString() << std::endl;
        delete item;
        this->removeItemWidget(item);
    }
}