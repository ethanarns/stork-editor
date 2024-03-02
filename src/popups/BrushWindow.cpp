#include "BrushWindow.h"
#include "../GlobalSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>

BrushWindow::BrushWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->setWindowTitle(tr("Brush Window"));
    this->setObjectName(tr("brushWindow"));
    auto mainLayout = new QVBoxLayout(this);

    this->brushTable = new BrushTable(this,rom);
    mainLayout->addWidget(this->brushTable);
    
    auto bar1 = new QHBoxLayout(this);
    auto flipHbox = new QCheckBox(this);
    flipHbox->setText(tr("Flip H"));
    bar1->addWidget(flipHbox);

    auto flipVbox = new QCheckBox(this);
    flipVbox->setText(tr("Flip V"));
    bar1->addWidget(flipVbox);

    mainLayout->addLayout(bar1);

    this->setLayout(mainLayout);
}