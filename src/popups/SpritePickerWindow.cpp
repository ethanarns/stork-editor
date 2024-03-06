#include "SpritePickerWindow.h"

#include "../utils.h"
#include "../GlobalSettings.h"

#include <sstream>

#include <QListWidget>
#include <QVBoxLayout>

SpritePickerWindow::SpritePickerWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setObjectName("spritePickerWindow");
    this->setWindowTitle(tr("Sprite Picker"));

    this->spriteList = new QListWidget(this);
    this->spriteList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    connect(this->spriteList,&QListWidget::currentItemChanged,this,&SpritePickerWindow::currentSpriteChanged);

    this->searchBox = new QLineEdit(this);
    this->searchBox->setPlaceholderText(tr("Search items"));
    connect(this->searchBox,&QLineEdit::textEdited,this,&SpritePickerWindow::searchTextChanged);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(this->searchBox);
    mainLayout->addWidget(this->spriteList);

    this->setLayout(mainLayout);
}

void SpritePickerWindow::updateSpriteList(QString filter) {
    while (this->spriteList->count() > 0) {
        delete this->spriteList->takeItem(0);
    }
    filter = filter.toLower();
    for (auto it = this->yidsRom->spriteMetadata.begin(); it != this->yidsRom->spriteMetadata.end(); it++) {
        std::stringstream ss;
        ss << "0x" << std::setw(3) << std::setfill('0') << std::hex << it->spriteId << " ";
        ss << it->name;
        QString spriteString(ss.str().c_str());
        // Should it be added?
        if (spriteString.contains(filter,Qt::CaseSensitivity::CaseInsensitive)) {
            auto item = new QListWidgetItem(spriteString);
            item->setData(0xff,(uint)it->spriteId);
            this->spriteList->addItem(item);
        }
    }
}

void SpritePickerWindow::searchTextChanged(const QString &text) {
    this->updateSpriteList(text);
}

void SpritePickerWindow::currentSpriteChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);
    if (current == nullptr) {
        YUtils::printDebug("Selected sprite list item was null",DebugType::ERROR);
        YUtils::popupAlert("Selected sprite list item was null");
        return;
    }
    globalSettings.currentSpriteIdToAdd = current->data(0xff).toUInt();
}
