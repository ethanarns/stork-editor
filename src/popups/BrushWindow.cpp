#include "BrushWindow.h"
#include "../GlobalSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

BrushWindow::BrushWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->setWindowTitle(tr("Brush Window"));
    this->setObjectName(tr("brushWindow"));
    auto mainLayout = new QVBoxLayout(this);

    this->brushTable = new BrushTable(this,rom);
    mainLayout->addWidget(this->brushTable);
    
    auto paletteAndFlip = new QHBoxLayout(this);

    mainLayout->addLayout(paletteAndFlip);

    this->setLayout(mainLayout);
}