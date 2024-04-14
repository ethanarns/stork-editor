#include "TriggerWindow.h"

#include <QWidget>
#include <QVBoxLayout>

TriggerWindow::TriggerWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setObjectName("triggerWindow");
    this->setWindowTitle(tr("Trigger Boxes"));

    this->triggerList = new QListWidget(this);
    this->triggerList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    auto mainLayout = new QHBoxLayout(this);

    auto settingsLayout = new QVBoxLayout(this);

    // Set up settings
    

    mainLayout->addWidget(this->triggerList);
    mainLayout->addLayout(settingsLayout);
    this->setLayout(mainLayout);
}