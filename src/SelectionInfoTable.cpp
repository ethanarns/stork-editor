#include "SelectionInfoTable.h"

#include "yidsrom.h"
#include "LevelObject.h"

#include <sstream>

#include <QtCore>
#include <QWidget>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

SelectionInfoTable::SelectionInfoTable(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setColumnCount(2);
    this->setRowCount(8);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();

    int i = 0;
    this->setText(0,i++,"ID",false);
    this->setText(0,i++,"UUID",false);
    this->setText(0,i++,"Name",false);
    this->setText(0,i++,"Description",false);
    this->setText(0,i++,"X Position",false);
    this->setText(0,i++,"Y Position",false);
    this->setText(0,i++,"Settings Length",false);
    this->setText(0,i++,"Settings",false);

    connect(this,&QTableWidget::cellDoubleClicked,this,&SelectionInfoTable::cellDoubleClicked);
    connect(this,&QTableWidget::itemChanged,this,&SelectionInfoTable::itemChanged);
}

void SelectionInfoTable::setText(int x, int y, std::string text, bool editable) {
    auto potentialExisting = this->item(y,x);
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets make a new one and set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setText(tr(text.c_str()));
        if (editable) {
            newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        } else {
            newItem->setFlags(Qt::ItemIsEnabled);
        }
        this->setItem(y,x,newItem);
    } else {
        potentialExisting->setText(tr(text.c_str()));
        if (editable) {
            potentialExisting->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        } else {
            potentialExisting->setFlags(Qt::ItemIsEnabled);
        }
    }
}

void SelectionInfoTable::updateWithLevelObject(LevelObject lo) {
    auto textMetadata = LevelObject::getObjectTextMetadata(lo.objectId);
    int i = 0;
    std::stringstream s0;
    s0 << "0x" << std::hex << lo.objectId;
    this->setText(1,i++,s0.str(),false);
    std::stringstream s1;
    s1 << "0x" << std::hex << lo.uuid;
    this->setText(1,i++,s1.str(),false);
    this->setText(1,i++,textMetadata.prettyName,false);
    this->setText(1,i++,textMetadata.description,false);
    std::stringstream sX;
    sX << "0x" << std::hex << lo.xPosition;
    std::stringstream sY;
    sY << "0x" << std::hex << lo.yPosition;
    this->setText(1,i++,sX.str(),true);
    this->setText(1,i++,sY.str(),true);
    // Settings
    std::stringstream sSettingsLength;
    sSettingsLength << "0x" << std::hex << lo.settingsLength;
    this->setText(1,i++,sSettingsLength.str(),false);
    std::stringstream ssSettings;
    if (lo.settingsLength != lo.settings.size()) {
        YUtils::printDebug("Settings length value and settings size not matching",DebugType::ERROR);
    }
    for (int j = 0; j < lo.settingsLength; j++) {
        ssSettings << std::hex << std::setw(2) << (uint16_t)lo.settings.at(j) << " ";
    }
    this->setText(1,i++,ssSettings.str(),true);
}

void SelectionInfoTable::cellDoubleClicked(int row, int column) {
    auto cell = this->item(row,column);
    if (cell == nullptr) {
        YUtils::printDebug("Cell double clicked is null",DebugType::WARNING);
        return;
    }
    if (cell->flags() & Qt::ItemIsEditable) {
        this->cellBeingEdited = cell;
    }
}

void SelectionInfoTable::itemChanged(QTableWidgetItem *item) {
    if (item == nullptr) {
        YUtils::printDebug("itemChanged given null item",DebugType::WARNING);
        return;
    }
    if (this->cellBeingEdited == nullptr) {
        // Fail silently, it will output a LOT
        return;
    }
    if (item == this->cellBeingEdited) {
        std::cout << item->text().toStdString() << std::endl;
        this->cellBeingEdited = nullptr;
    }
}
