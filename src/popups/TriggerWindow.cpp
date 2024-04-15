#include "TriggerWindow.h"

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
    auto row1spin = new QSpinBox(this);
    row1->addWidget(row1spin);

    // Row 2 (Top Y)
    auto row2 = new QHBoxLayout(this);
    settingsLayout->addLayout(row2);
    auto row2text = new QLabel(tr("Top Y"));
    row2->addWidget(row2text);
    auto row2spin = new QSpinBox(this);
    row2->addWidget(row2spin);

    // Row 3 (Right X)
    auto row3 = new QHBoxLayout(this);
    settingsLayout->addLayout(row3);
    auto row3text = new QLabel(tr("Right X"));
    row3->addWidget(row3text);
    auto row3spin = new QSpinBox(this);
    row3->addWidget(row3spin);

    // Row 4 (Bottom Y)
    auto row4 = new QHBoxLayout(this);
    settingsLayout->addLayout(row4);
    auto row4text = new QLabel(tr("Bottom Y"));
    row4->addWidget(row4text);
    auto row4spin = new QSpinBox(this);
    row4->addWidget(row4spin);

    // Row 5 (Add and delete)
    auto row5 = new QHBoxLayout(this);
    settingsLayout->addLayout(row5);
    auto addButton = new QPushButton(tr("Add"),this);
    row5->addWidget(addButton);
    auto deleteButton = new QPushButton(tr("Delete"),this);
    row5->addWidget(deleteButton);

    mainLayout->addWidget(this->triggerList);
    mainLayout->addLayout(settingsLayout);
    this->setLayout(mainLayout);
}