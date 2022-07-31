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
}

void LevelSelect::updateList() {
    if (!this->yidsRom->filesLoaded) {
        std::cerr << "[ERROR] Could not update LevelSelect before ROM is loaded" << std::endl;
        return;
    }
    for (int worldIndex = 0; worldIndex < 5; worldIndex++) {
        for (int levelIndex = 0; levelIndex < 10; levelIndex++) {
            auto curLevelCrsbName = this->yidsRom->getLevelFileNameFromMapIndex(worldIndex,levelIndex);
            QListWidgetItem* item = new QListWidgetItem(tr(curLevelCrsbName.c_str()));
            item->setData(LevelSelect::ITEM_DATA_ID_CRSB,tr(curLevelCrsbName.c_str()));
            item->setData(LevelSelect::ITEM_DATA_ID_LEVEL,levelIndex);
            item->setData(LevelSelect::ITEM_DATA_ID_WORLD,worldIndex);
            this->addItem(item);
        }
    }
}

int LevelSelect::wipeList() {
    int deletedCount = 0;
    auto len = this->count();
    for (int i = 0; i < len; i++) {
        auto curItem = this->item(i);
        delete curItem;
        this->removeItemWidget(curItem);
        deletedCount++;
    }
    return deletedCount;
}