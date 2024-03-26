#include "BrushWindow.h"
#include "../GlobalSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFile>
#include <QLineEdit>

BrushWindow::BrushWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->setWindowTitle(tr("Brush Window"));
    this->setObjectName(tr("brushWindow"));
    this->setMinimumHeight(301);
    this->resize(417,301);
    this->setMinimumWidth(417);
    auto mainLayout = new QHBoxLayout(this);
    auto rightLayout = new QVBoxLayout(this);
    auto leftLayout = new QVBoxLayout(this);

    this->brushTable = new BrushTable(this,rom);
    this->brushTable->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    leftLayout->addWidget(this->brushTable);
    
    // Row 1
    auto bar1 = new QHBoxLayout(this);
    auto flipHbox = new QCheckBox(this);
    flipHbox->setText(tr("Flip H"));
    connect(flipHbox,&QCheckBox::stateChanged,this,&BrushWindow::stateChangedH);
    bar1->addWidget(flipHbox);

    auto flipVbox = new QCheckBox(this);
    flipVbox->setText(tr("Flip V"));
    connect(flipVbox,&QCheckBox::stateChanged,this,&BrushWindow::stateChangedV);
    bar1->addWidget(flipVbox);

    // Row 2
    auto bar2 = new QHBoxLayout(this);

    auto loadFromSelectionButton = new QPushButton("&Load Selected",this);
    connect(loadFromSelectionButton,&QPushButton::released, this, &BrushWindow::loadSelectionClicked);
    bar2->addWidget(loadFromSelectionButton);

    auto clearBrushButton = new QPushButton("&Clear Brush",this);
    connect(clearBrushButton,&QPushButton::released, this, &BrushWindow::clearBrushClicked);
    bar2->addWidget(clearBrushButton);

    // Row 3 //
    auto bar3 = new QHBoxLayout(this);
    // Brush name
    this->textboxBrushName = new QLineEdit(this);
    this->textboxBrushName->setObjectName("textboxBrushName");
    this->textboxBrushName->setText("brush1");
    // ConnectMe
    bar3->addWidget(this->textboxBrushName);
    // Save brush
    auto saveBrush = new QPushButton("&Save Brush",this);
    saveBrush->setObjectName("buttonSaveBrushYdb");
    connect(saveBrush,&QPushButton::released,this,&BrushWindow::saveBrushClicked);
    bar3->addWidget(saveBrush);

    leftLayout->addLayout(bar1);
    leftLayout->addLayout(bar2);
    leftLayout->addLayout(bar3);
    mainLayout->addLayout(leftLayout);

    // Right side //
    this->stampList = new QListWidget(this);
    connect(this->stampList,&QListWidget::currentRowChanged,this,&BrushWindow::stampListSelectedRowChanged);
    rightLayout->addWidget(this->stampList);

    auto listButtonBarLayout = new QHBoxLayout(this);
    auto deleteButton = new QPushButton("Delete",this);
    connect(deleteButton,&QPushButton::released,this,&BrushWindow::deleteSelectedBrush);
    listButtonBarLayout->addWidget(deleteButton);
    auto loadBrushButton = new QPushButton("Load",this);
    connect(loadBrushButton,&QPushButton::released,this,&BrushWindow::loadBrushFile);
    listButtonBarLayout->addWidget(loadBrushButton);
    rightLayout->addLayout(listButtonBarLayout);

    mainLayout->addLayout(rightLayout);

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

void BrushWindow::loadSelectionClicked() {
    int whichTileAttrData = 0;
    if (globalSettings.currentEditingBackground == 1) {
        whichTileAttrData = PixelDelegateData::TILEATTR_BG1;
    } else if (globalSettings.currentEditingBackground == 2) {
        whichTileAttrData = PixelDelegateData::TILEATTR_BG2;
    } else if (globalSettings.currentEditingBackground == 3) {
        whichTileAttrData = PixelDelegateData::TILEATTR_BG3;
    } else {
        YUtils::printDebug("Cannot load selection in non-BG mode",DebugType::WARNING);
        YUtils::popupAlert("Cannot load selection in non-BG mode");
        return;
    }
    this->brushTable->resetTable();
    auto tiles = globalSettings.selectedItemPointers;
    globalSettings.currentBrush->tileAttrs.clear();
    auto width = globalSettings.getSelectionWidth();
    if (width > BrushTable::CELL_COUNT_DIMS) {
        YUtils::popupAlert("Selection width too big for brush");
        return;
    }
    if (width == 0) {
        YUtils::printDebug("Attempting to divided by zero in loadSelectionClicked",DebugType::WARNING);
        YUtils::popupAlert("Selection width was 0");
        return;
    }
    for (uint tileIndex = 0; tileIndex < tiles.size(); tileIndex++) {
        auto tile = tiles.at(tileIndex);
        uint16_t tileAttr = (uint16_t)tile->data(whichTileAttrData).toUInt();
        auto mapTile = YUtils::getMapTileRecordDataFromShort(tileAttr);
        int row = tileIndex / width;
        int col = tileIndex % width;
        this->brushTable->setTile(row,col,mapTile);
    }
    this->brushTable->loadTilesToCurBrush();
}

void BrushWindow::clearBrushClicked() {
    this->brushTable->resetTable();
    // No pointers, so no need to delete[]
    globalSettings.currentBrush->tileAttrs.clear();
    this->updateStampList();
}

bool BrushWindow::saveCurrentBrushToFile() {
    auto json = globalSettings.currentBrush->toJson();
    std::string saveName = globalSettings.currentBrush->name.append(".ydb");

    QFile saveFile(saveName.c_str());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        YUtils::printDebug("Could not save brush file, write only",DebugType::ERROR);
        YUtils::popupAlert("Could not save brush file, write only");
        return false;
    }
    saveFile.write(QJsonDocument(json).toJson());
    return true;
}

bool BrushWindow::loadFileToCurrentBrush(std::string filename) {
    if (globalSettings.currentEditingBackground == 0) {
        YUtils::printDebug("Cannot load brush file while not in BG mode",DebugType::WARNING);
        YUtils::popupAlert("Cannot load brush file while not in BG mode");
        return false;
    }
    QFile loadFile(filename.c_str());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        YUtils::printDebug("Could not load brush file, read only",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush file, read only");
        return false;
    }
    QByteArray fileData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(fileData));
    QJsonObject json = loadDoc.object();
    if (const QJsonValue brushTileset = json["brushTileset"]; brushTileset.isString()) {
        globalSettings.currentBrush->brushTileset = brushTileset.toString().toStdString();
    }
    if (const QJsonValue tileAttrsMaybe = json["tileAttrs"]; tileAttrsMaybe.isArray()) {
        const QJsonArray tileAttrs = tileAttrsMaybe.toArray();
        globalSettings.currentBrush->tileAttrs.clear();
        globalSettings.currentBrush->tileAttrs.reserve(tileAttrs.size());
        for (const QJsonValue &attrData : tileAttrs) {
            auto attr = (uint16_t)attrData.toInt();
            auto attrTile = YUtils::getMapTileRecordDataFromShort(attr);
            globalSettings.currentBrush->tileAttrs.push_back(attrTile);
        }
    }
    this->brushTable->updateBrushDims();
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    std::map<uint32_t,Chartile> tilesMap = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    uint8_t paletteOffset = (uint8_t)this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    for (int y = 0; y < this->brushTable->rowCount(); y++) {
        for (int x = 0; x < this->brushTable->columnCount(); x++) {
            auto item = this->brushTable->item(y,x);
            uint32_t index = x + (y*this->brushTable->columnCount());
            auto mapTile = globalSettings.currentBrush->tileAttrs.at(index);
            if (item == nullptr) {
                item = new QTableWidgetItem();
                this->brushTable->setItem(y,x,item);
                item->setData(PixelDelegateData::DRAW_BG1,true);
                item->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
            }
            auto pal = mapTile.paletteId + paletteOffset;
            item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(mapTile.tileId).tiles);
            item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[pal]);
            item->setData(PixelDelegateData::TILE_ID_BG1,mapTile.tileId);
            item->setData(PixelDelegateData::PALETTE_ID_BG1,pal);
            item->setData(PixelDelegateData::FLIP_H_BG1,mapTile.flipH);
            item->setData(PixelDelegateData::FLIP_V_BG1,mapTile.flipV);
        }
    }
    return true;
}

void BrushWindow::updateStampList() {
    // Wipe it
    while (this->stampList->count()) {
        delete this->stampList->takeItem(0);
    }
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    if (scen == nullptr) {
        YUtils::printDebug("Invalid layer when updating stamp list",DebugType::WARNING);
        return;
    }
    auto currentScenImbz = scen->getInfo()->imbzFilename;
    // Fill it
    for (auto bit = globalSettings.brushes.begin(); bit != globalSettings.brushes.end(); bit++) {
        auto curBrush = *bit;
        auto newItem = new QListWidgetItem(this->stampList);
        auto brushImbz = curBrush.brushTileset;
        if (brushImbz.compare(currentScenImbz) != 0) {
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEnabled);
        }
        newItem->setText(QString::fromStdString(curBrush.name));
        this->stampList->addItem(newItem);
    }
}

void BrushWindow::saveBrushClicked() {
    if (globalSettings.currentEditingBackground == 0) {
        YUtils::popupAlert("Cannot save brushes on this layer");
        YUtils::printDebug("saveBrushClicked detected bg 0",DebugType::WARNING);
        return;
    }
    auto curBg = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    if (curBg == nullptr) {
        YUtils::printDebug("saveBrushClicked bg is null",DebugType::WARNING);
        return;
    }
    auto imbzFilename = curBg->getInfo()->imbzFilename;
    if (imbzFilename.empty()) {
        YUtils::popupAlert("Embedded IMBZs not yet supported");
        YUtils::printDebug("imbzFilename is empty, unsupported",DebugType::WARNING);
        return;
    }
    globalSettings.currentBrush->brushTileset = imbzFilename;
    auto brushNameText = this->textboxBrushName->text().trimmed().toStdString();
    if (brushNameText.empty()) {
        YUtils::popupAlert("Please enter a brush name to save");
        return;
    }
    globalSettings.currentBrush->name = brushNameText;
    globalSettings.brushes.push_back(*globalSettings.currentBrush);

    this->updateStampList();
    this->textboxBrushName->clear();
}

void BrushWindow::stampListSelectedRowChanged(int currentRow) {
    if (currentRow == -1) {
       return;
    }
    if (globalSettings.brushes.size() == 0) {
        YUtils::printDebug("Brush list is empty, mismatch",DebugType::ERROR);
        YUtils::popupAlert("Brush list is empty, list mismatch");
        return;
    }
    if ((uint)currentRow >= globalSettings.brushes.size()) {
        YUtils::printDebug("Brush list widget has too many brushes",DebugType::ERROR);
        YUtils::popupAlert("Brush list widget has too many brushes");
        return;
    }
    YUtils::printDebug("stampListSelectedRowChanged");
    auto selectedStampData = globalSettings.brushes.at(currentRow);
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    if (scen == nullptr) {
        YUtils::printDebug("Invalid layer when selecting stamp",DebugType::WARNING);
        return;
    }
    if (scen->getInfo()->imbzFilename.compare(selectedStampData.brushTileset) != 0) {
        std::stringstream ssMismatchImbz;
        ssMismatchImbz << "Mismatch in current tileset vs selected: ";
        ssMismatchImbz << scen->getInfo()->imbzFilename << " vs ";
        ssMismatchImbz << selectedStampData.brushTileset;
        YUtils::printDebug(ssMismatchImbz.str(),DebugType::WARNING);
        return;
    }
    // Change current data in brush
    *globalSettings.currentBrush = selectedStampData;
    auto tiles = selectedStampData.tileAttrs;
    this->brushTable->resetTable();
    uint width = 12; // temp, brushes should be size agnostic (TODO)
    for (uint tileIndex = 0; tileIndex < tiles.size(); tileIndex++) {
        auto mapTile = tiles.at(tileIndex);
        int row = tileIndex / width;
        int col = tileIndex % width;
        this->brushTable->setTile(row,col,mapTile);
    }
    this->brushTable->loadTilesToCurBrush();
    this->textboxBrushName->setText(QString::fromStdString(selectedStampData.name));
}

void BrushWindow::deleteSelectedBrush() {
    YUtils::printDebug("deleteSelectedBrush");
}

void BrushWindow::loadBrushFile() {
    YUtils::printDebug("loadBrushFile");
}
