#include "GuiObjectList.h"
#include "yidsrom.h"
#include "LevelObject.h"

#include <QtCore>
#include <QWidget>

GuiObjectList::GuiObjectList(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
}

void GuiObjectList::updateList() {
    this->wipeList();
    this->clear();
    for (
        auto objIt = this->yidsRom->loadedLevelObjects.begin();
        objIt != this->yidsRom->loadedLevelObjects.end();
        ++objIt
    ) {
        ObjectTextMetadata textData = LevelObject::getObjectTextMetadata(objIt->objectId);
        QListWidgetItem* item = new QListWidgetItem(tr(textData.prettyName.c_str()));
        item->setData(GuiObjectList::LEVEL_OBJECT_UUID,objIt->uuid);
        this->addItem(item);
    }
}

int GuiObjectList::wipeList() {
    int deletedCount = 0;
    auto len = this->count();
    // Since you're deleting things, do not 
    for (int i = 0; i < len; i++) {
        auto curItem = this->item(i);
        delete curItem;
        this->removeItemWidget(curItem);
        deletedCount++;
    }
    return deletedCount;
}