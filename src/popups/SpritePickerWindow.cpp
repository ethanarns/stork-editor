#include "SpritePickerWindow.h"

#include <QListWidget>
#include <QVBoxLayout>

SpritePickerWindow::SpritePickerWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->spriteList = new QListWidget(this);
    this->searchBox = new QLineEdit(this);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(this->searchBox);
    mainLayout->addWidget(this->spriteList);

    this->setLayout(mainLayout);
}
