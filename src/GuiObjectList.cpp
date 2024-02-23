#include "GuiObjectList.h"
#include "yidsrom.h"
#include "LevelObject.h"

#include <QtCore>
#include <QWidget>
#include <QListWidget>
#include <QAbstractItemView>

GuiObjectList::GuiObjectList(QWidget* parent, YidsRom* rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;

    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
}

void GuiObjectList::updateList() {
    YUtils::printDebug("Updating GUI sprite list",DebugType::VERBOSE);
    this->wipeList();
    this->clear();
    auto loadedLevelObjects = this->yidsRom->mapData->getAllLevelObjects();
    for (
        auto objItP = loadedLevelObjects.begin();
        objItP != loadedLevelObjects.end();
        ++objItP
    ) {
        auto objIt = (*objItP);
        ObjectTextMetadata textData = LevelObject::getObjectTextMetadata(objIt->objectId);
        QListWidgetItem* item = new QListWidgetItem(tr(textData.prettyName.c_str()));
        item->setData(GuiObjectList::LEVEL_OBJECT_UUID,objIt->uuid);
        item->setData(GuiObjectList::LEVEL_OBJECT_ID,objIt->objectId);
        this->addItem(item);
    }
}

int GuiObjectList::wipeList() {
    int deletedCount = 0;
    auto len = this->count();
    for (int i = 0; i < len; i++) {
        auto curItem = this->item(i);
        delete curItem;
        deletedCount++;
    }
    this->clear();
    return deletedCount;
}