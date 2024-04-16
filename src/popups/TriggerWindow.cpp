#include "TriggerWindow.h"
#include "../DisplayTable.h"

#include <iostream>
#include <sstream>

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>

TriggerWindow::TriggerWindow(QWidget *parent, YidsRom *rom, DisplayTable *grid) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->grid = grid;

    this->setObjectName("triggerWindow");
    this->setWindowTitle(tr("Trigger Boxes"));
    this->setMaximumWidth(400);

    this->triggerList = new QListWidget(this);
    this->triggerList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    connect(this->triggerList,&QListWidget::currentRowChanged,this,&TriggerWindow::triggerListSelectionChanged);

    auto mainLayout = new QHBoxLayout(this);

    auto settingsLayout = new QVBoxLayout(this);

    // Row 1 (Left X)
    auto row1 = new QHBoxLayout(this);
    settingsLayout->addLayout(row1);
    auto row1text = new QLabel(tr("Left X"));
    row1->addWidget(row1text);
    this->leftX = new QSpinBox(this);
    this->leftX->setMaximum(0xffff);
    this->leftX->setDisplayIntegerBase(16);
    this->leftX->setPrefix(tr("0x"));
    row1->addWidget(this->leftX);
    connect(this->leftX,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 2 (Top Y)
    auto row2 = new QHBoxLayout(this);
    settingsLayout->addLayout(row2);
    auto row2text = new QLabel(tr("Top Y"));
    row2->addWidget(row2text);
    this->topY = new QSpinBox(this);
    this->topY->setMaximum(0xffff);
    this->topY->setDisplayIntegerBase(16);
    this->topY->setPrefix(tr("0x"));
    row2->addWidget(this->topY);
    connect(this->topY,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 3 (Right X)
    auto row3 = new QHBoxLayout(this);
    settingsLayout->addLayout(row3);
    auto row3text = new QLabel(tr("Right X"));
    row3->addWidget(row3text);
    this->rightX = new QSpinBox(this);
    this->rightX->setMaximum(0xffff);
    this->rightX->setDisplayIntegerBase(16);
    this->rightX->setPrefix(tr("0x"));
    row3->addWidget(this->rightX);
    connect(this->rightX,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 4 (Bottom Y)
    auto row4 = new QHBoxLayout(this);
    settingsLayout->addLayout(row4);
    auto row4text = new QLabel(tr("Bottom Y"));
    row4->addWidget(row4text);
    this->bottomY = new QSpinBox(this);
    this->bottomY->setMaximum(0xffff);
    this->bottomY->setDisplayIntegerBase(16);
    this->bottomY->setPrefix(tr("0x"));
    row4->addWidget(this->bottomY);
    connect(this->bottomY,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 5 (Add and delete)
    auto row5 = new QHBoxLayout(this);
    settingsLayout->addLayout(row5);
    auto addButton = new QPushButton(tr("Add"),this);
    connect(addButton,&QPushButton::pressed,this,&TriggerWindow::addNewTrigger);
    row5->addWidget(addButton);
    auto deleteButton = new QPushButton(tr("Delete"),this);
    connect(deleteButton,&QPushButton::pressed,this,&TriggerWindow::deleteTrigger);
    row5->addWidget(deleteButton);

    mainLayout->addWidget(this->triggerList);
    mainLayout->addLayout(settingsLayout);
    this->setLayout(mainLayout);
    this->allowChanges = true;
}

void TriggerWindow::updateTriggerList() {
    // Wipe first
    while (this->triggerList->count() > 0) {
        delete this->triggerList->takeItem(0);
    }
    auto areaMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
    if (areaMaybe == nullptr) {
        //YUtils::printDebug("AREA not found");
        return;
    }
    this->allowChanges = false;
    //YUtils::printDebug("Updating TriggerBox list");
    auto area = static_cast<TriggerBoxData*>(areaMaybe);
    uint i = 0;
    for (auto it = area->triggers.begin(); it != area->triggers.end(); it++) {
        std::stringstream ss;
        ss << "Trigger 0x" << std::hex << i++;
        this->triggerList->addItem(QString::fromStdString(ss.str()));
    }
    this->allowChanges = true;
}

void TriggerWindow::spinboxValueChanged(int i) {
    //YUtils::printDebug("spinboxValueChanged");
    Q_UNUSED(i);
    if (!this->allowChanges) {
        //YUtils::printDebug("Could not make TriggerBox change, disabled",DebugType::WARNING);
        return;
    }
    int lx = this->leftX->value();
    int ty = this->topY->value();
    int rx = this->rightX->value();
    int by = this->bottomY->value();
    auto curRowIndex = this->triggerList->currentRow();
    if (curRowIndex < 0) {
        YUtils::printDebug("No TriggerBox selected",DebugType::WARNING);
        return;
    }
    // Get all triggers
    auto areaMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
    if (areaMaybe == nullptr) {
        YUtils::printDebug("Failed to get TriggerBoxes for level",DebugType::WARNING);
        return;
    }
    auto triggers = static_cast<TriggerBoxData*>(areaMaybe)->triggers;
    if ((uint)curRowIndex >= triggers.size()) {
        YUtils::printDebug("Attempting to retrieve TriggerBox with too high an index",DebugType::ERROR);
        return;
    }
    auto selectedTrigger = triggers.at(curRowIndex);
    selectedTrigger->leftX = (uint16_t)lx;
    selectedTrigger->topY = (uint16_t)ty;
    selectedTrigger->rightX = (uint16_t)rx;
    selectedTrigger->bottomY = (uint16_t)by;
    this->grid->updateTriggerBoxes(curRowIndex);
    emit this->markSavableChange();
}

void TriggerWindow::triggerListSelectionChanged(int currentRow) {
    //YUtils::printDebug("triggerListSelectionChanged");
    if (currentRow < 0) {
        return;
    }
    // Get all triggers
    auto areaMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
    if (areaMaybe == nullptr) {
        YUtils::printDebug("Failed to get TriggerBoxes for level",DebugType::WARNING);
        return;
    }
    auto triggers = static_cast<TriggerBoxData*>(areaMaybe)->triggers;
    if ((uint)currentRow >= triggers.size()) {
        YUtils::printDebug("Attempting to retrieve TriggerBox with too high an index",DebugType::ERROR);
        return;
    }
    auto selectedTrigger = triggers.at(currentRow);
    std::stringstream ss;
    ss << "Selected TriggerBox with UUID 0x" << std::hex << selectedTrigger->uuid;
    YUtils::printDebug(ss.str());
    this->allowChanges = false;
    this->leftX->setValue(selectedTrigger->leftX);
    this->topY->setValue(selectedTrigger->topY);
    this->rightX->setValue(selectedTrigger->rightX);
    this->bottomY->setValue(selectedTrigger->bottomY);
    this->allowChanges = true;
    this->grid->updateTriggerBoxes(currentRow);
}

void TriggerWindow::addNewTrigger() {
    //YUtils::printDebug("addNewTrigger");
    auto areaMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
    if (areaMaybe == nullptr) {
        YUtils::printDebug("AREA data not found, creating new");
        auto area = new TriggerBoxData();
        this->yidsRom->mapData->subData.push_back(area);
        areaMaybe = area;
    }
    auto trueArea = static_cast<TriggerBoxData*>(areaMaybe);
    auto newTrigger = new TriggerBox();
    newTrigger->leftX = 4;
    newTrigger->topY = 4;
    newTrigger->rightX = 8;
    newTrigger->bottomY = 8;
    uint32_t newUuid = 1;
    if (trueArea->triggers.size() > 0) {
        newUuid = trueArea->triggers.at(trueArea->triggers.size()-1)->uuid + 1;
    }
    newTrigger->uuid = newUuid;
    trueArea->triggers.push_back(newTrigger);
    // Update, selecting the new one
    this->updateTriggerList();
    this->grid->updateTriggerBoxes();
    emit this->markSavableChange();
}

void TriggerWindow::deleteTrigger() {
    auto currentSelectedRow = triggerList->currentRow();
    if (currentSelectedRow < 0) {
        YUtils::popupAlert("No TriggerBox selected");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this,"Delete TriggerBox","Warning: TriggerBoxes are targeted by index, meaning position matters. Deleting WILL break connections. Are you sure?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        YUtils::printDebug("Deleting selected TriggerBox");
        auto areaMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
        if (areaMaybe == nullptr) {
            YUtils::printDebug("Failed to get TriggerBoxes for level",DebugType::WARNING);
            return;
        }
        auto triggers = &static_cast<TriggerBoxData*>(areaMaybe)->triggers;
        triggers->erase(std::next(triggers->begin(),currentSelectedRow));
        this->updateTriggerList();
        this->grid->updateTriggerBoxes();
        emit this->markSavableChange();
    } else {
        YUtils::printDebug("Canceling delete TriggerBox");
    }
}
