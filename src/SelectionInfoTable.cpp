#include "SelectionInfoTable.h"

#include "yidsrom.h"
#include "LevelObject.h"

#include <sstream>

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

    this->setText(0,0,"ID",false);
    this->setText(0,1,"UUID",false);
    this->setText(0,2,"Name",false);
    this->setText(0,3,"Description",false);
}

void SelectionInfoTable::setText(int x, int y, std::string text, bool editable) {
    auto potentialExisting = this->item(y,x);
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets make a new one and set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setText(tr(text.c_str()));
        newItem->setFlags(Qt::ItemIsEnabled);
        this->setItem(y,x,newItem);
    } else {
        potentialExisting->setText(tr(text.c_str()));
        potentialExisting->setFlags(Qt::ItemIsEnabled);
    }
}

void SelectionInfoTable::updateWithLevelObject(LevelObject lo) {
    auto textMetadata = LevelObject::getObjectTextMetadata(lo.objectId);
    std::stringstream s0;
    s0 << std::hex << lo.objectId;
    this->setText(1,0,s0.str(),false);
    std::stringstream s1;
    s1 << lo.uuid;
    this->setText(1,1,s1.str(),false);
    this->setText(1,2,textMetadata.prettyName,false);
    this->setText(1,3,textMetadata.description,false);
}