#include "TriggerWindow.h"

#include <iostream>
#include <sstream>

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

TriggerWindow::TriggerWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setObjectName("triggerWindow");
    this->setWindowTitle(tr("Trigger Boxes"));

    this->triggerList = new QListWidget(this);
    this->triggerList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    auto mainLayout = new QHBoxLayout(this);

    auto settingsLayout = new QVBoxLayout(this);

    // Row 1 (Left X)
    auto row1 = new QHBoxLayout(this);
    settingsLayout->addLayout(row1);
    auto row1text = new QLabel(tr("Left X"));
    row1->addWidget(row1text);
    this->leftX = new QSpinBox(this);
    row1->addWidget(this->leftX);
    connect(this->leftX,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 2 (Top Y)
    auto row2 = new QHBoxLayout(this);
    settingsLayout->addLayout(row2);
    auto row2text = new QLabel(tr("Top Y"));
    row2->addWidget(row2text);
    this->topY = new QSpinBox(this);
    row2->addWidget(this->topY);
    connect(this->topY,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 3 (Right X)
    auto row3 = new QHBoxLayout(this);
    settingsLayout->addLayout(row3);
    auto row3text = new QLabel(tr("Right X"));
    row3->addWidget(row3text);
    this->rightX = new QSpinBox(this);
    row3->addWidget(this->rightX);
    connect(this->rightX,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 4 (Bottom Y)
    auto row4 = new QHBoxLayout(this);
    settingsLayout->addLayout(row4);
    auto row4text = new QLabel(tr("Bottom Y"));
    row4->addWidget(row4text);
    this->bottomY = new QSpinBox(this);
    row4->addWidget(this->bottomY);
    connect(this->bottomY,QOverload<int>::of(&QSpinBox::valueChanged),this,&TriggerWindow::spinboxValueChanged);

    // Row 5 (Add and delete)
    auto row5 = new QHBoxLayout(this);
    settingsLayout->addLayout(row5);
    auto addButton = new QPushButton(tr("Add"),this);
    addButton->setDisabled(true);
    row5->addWidget(addButton);
    auto deleteButton = new QPushButton(tr("Delete"),this);
    deleteButton->setDisabled(true);
    row5->addWidget(deleteButton);

    mainLayout->addWidget(this->triggerList);
    mainLayout->addLayout(settingsLayout);
    this->setLayout(mainLayout);
}

void TriggerWindow::updateTriggerList() {
    auto areaMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
    if (areaMaybe == nullptr) {
        return;
    }
    YUtils::printDebug("Updating TriggerBox list");
    while (this->triggerList->count() > 0) {
        delete this->triggerList->takeItem(0);
    }
    auto area = static_cast<TriggerBoxData*>(areaMaybe);
    uint i = 0;
    for (auto it = area->triggers.begin(); it != area->triggers.end(); it++) {
        std::stringstream ss;
        ss << "Trigger 0x" << std::hex << i++;
        this->triggerList->addItem(QString::fromStdString(ss.str()));
    }
}

void TriggerWindow::spinboxValueChanged(int i) {
    YUtils::printDebug("spinboxValueChanged");
    Q_UNUSED(i);
    int lx = this->leftX->value();
    int ty = this->topY->value();
    int rx = this->rightX->value();
    int by = this->bottomY->value();
}
