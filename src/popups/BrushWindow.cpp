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
    connect(flipHbox,&QCheckBox::stateChanged,this,&BrushWindow::stateChangedH);
    bar1->addWidget(flipHbox);

    auto flipVbox = new QCheckBox(this);
    flipVbox->setText(tr("Flip V"));
    connect(flipVbox,&QCheckBox::stateChanged,this,&BrushWindow::stateChangedV);
    bar1->addWidget(flipVbox);

    mainLayout->addLayout(bar1);

    this->setLayout(mainLayout);
}

void BrushWindow::stateChangedH(int state) {
    if (state == 0) {
        globalSettings.brushFlipH = false;
    } else if (state == 1) {
        YUtils::printDebug("Somehow, stateChangedH has a 1 (Qt::PartiallyChecked)",DebugType::ERROR);
    } else if (state == 2) {
        globalSettings.brushFlipH = true;
    } else {
        YUtils::printDebug("Somehow, stateChangedH has a weird number",DebugType::ERROR);
    }
}

void BrushWindow::stateChangedV(int state) {
    if (state == 0) {
        globalSettings.brushFlipV = false;
    } else if (state == 1) {
        YUtils::printDebug("Somehow, stateChangedV has a 1 (Qt::PartiallyChecked)",DebugType::ERROR);
    } else if (state == 2) {
        globalSettings.brushFlipV = true;
    } else {
        YUtils::printDebug("Somehow, stateChangedV has a weird number",DebugType::ERROR);
    }
}
