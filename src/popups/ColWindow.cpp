#include "ColWindow.h"

#include <QWidget>
#include <QHBoxLayout>

ColWindow::ColWindow(QWidget *parent, YidsRom *rom) {
    this->yidsRom = rom;
    Q_UNUSED(parent);
    this->setWindowTitle(tr("Collision Window"));
    this->setObjectName(tr("colWindow"));
    this->setMinimumWidth(200);

    auto mainLayout = new QVBoxLayout(this);
    this->setLayout(mainLayout);

    this->table = new ColTable(this);
    mainLayout->addWidget(this->table);
}