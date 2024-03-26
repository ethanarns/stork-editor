#include "BrushWindow.h"
#include "../GlobalSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFile>
#include <QLineEdit>
#include <QFileDialog>

#include <filesystem>

BrushWindow::BrushWindow(QWidget *parent, YidsRom *rom) {
    Q_UNUSED(parent);
    this->yidsRom = rom;
    this->setWindowTitle(tr("Brush Window"));
    this->setObjectName(tr("brushWindow"));
    this->setMinimumHeight(365);
    this->resize(543,365);
    this->setMinimumWidth(543);
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
    saveBrush->setObjectName("buttonSaveBrushToList");
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

    // Button list
    auto listButtonBarLayout = new QHBoxLayout(this);
    auto deleteButton = new QPushButton("Delete",this);
    connect(deleteButton,&QPushButton::released,this,&BrushWindow::deleteSelectedBrush);
    listButtonBarLayout->addWidget(deleteButton);
    auto loadBrushButton = new QPushButton("Load",this);
    connect(loadBrushButton,&QPushButton::released,this,&BrushWindow::loadBrushFile);
    listButtonBarLayout->addWidget(loadBrushButton);
    auto exportBrushesButton = new QPushButton("Export",this);
    connect(exportBrushesButton,&QPushButton::released,this,&BrushWindow::exportBrush);
    listButtonBarLayout->addWidget(exportBrushesButton);

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
    globalSettings.currentBrush->brushWidth = width;
    // Add current palette offset
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    if (scen == nullptr) {
        YUtils::printDebug("Could not find bg in loadSelectionClicked",DebugType::ERROR);
        return;
    }
    int paletteOffset = (int)this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    globalSettings.currentBrush->paletteOffset = paletteOffset;
}

void BrushWindow::clearBrushClicked() {
    this->brushTable->resetTable();
    // No pointers, so no need to delete[]
    globalSettings.currentBrush->tileAttrs.clear();
    this->textboxBrushName->clear();
    this->updateStampList();
}

bool BrushWindow::saveCurrentBrushToFile() {
    auto json = globalSettings.currentBrush->toJson();
    auto fileName = QFileDialog::getSaveFileName(this,tr("Save brush"),".",tr("YIDS Brush files (*.ydb)"));
    if (fileName.isEmpty()) {
        YUtils::printDebug("Empty filename, canceling save current brush",DebugType::WARNING);
        return false;
    }
    if (!fileName.endsWith(".ydb")) {
        fileName.append(".ydb");
    }

    QFile saveFile(fileName);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        YUtils::printDebug("Could not save brush file, write only",DebugType::ERROR);
        YUtils::popupAlert("Could not save brush file, write only");
        return false;
    }
    std::stringstream ss;
    ss << "Saving brush file '" << fileName.toStdString() << "'";
    YUtils::printDebug(ss.str()); 
    saveFile.write(QJsonDocument(json).toJson());
    return true;
}

bool BrushWindow::loadBrushFileToList(std::string filePath) {
    if (globalSettings.currentEditingBackground == 0) {
        YUtils::printDebug("Cannot load brush file while not in BG mode",DebugType::WARNING);
        YUtils::popupAlert("Cannot load brush file while not in BG mode");
        return false;
    }

    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    if (scen == nullptr) {
        YUtils::printDebug("BG was null in loadBrushFileToList",DebugType::ERROR);
        return false;
    }

    QFile loadFile(filePath.c_str());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        YUtils::printDebug("Could not load brush file, read only",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush file, read only");
        return false;
    }
    QByteArray fileData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(fileData));
    QJsonObject json = loadDoc.object();

    // Brush tileset name
    std::string newBrushTileset = "error";
    if (const QJsonValue brushTileset = json["brushTileset"]; brushTileset.isString()) {
        auto loadedTilesetName = brushTileset.toString().toStdString();
        if (scen->getInfo()->imbzFilename.compare(loadedTilesetName) != 0) {
            std::stringstream ssImbzMismatch;
            ssImbzMismatch << "Tileset mismatch in loaded brush tileset vs current bg tileset: '";
            ssImbzMismatch << loadedTilesetName << "' vs '" << scen->getInfo()->imbzFilename << "'";
            YUtils::printDebug(ssImbzMismatch.str(),DebugType::ERROR);
            YUtils::popupAlert(ssImbzMismatch.str());
            return false;
        }
        newBrushTileset = loadedTilesetName;
    } else {
        YUtils::printDebug("Could not load brush, missing 'brushTileset'",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush, missing 'brushTileset'");
        return false;
    }

    // Tile attribute array (width not yet needed)
    std::vector<MapTileRecordData> newTileAttrs;
    if (const QJsonValue tileAttrsMaybe = json["tileAttrs"]; tileAttrsMaybe.isArray()) {
        const QJsonArray tileAttrs = tileAttrsMaybe.toArray();
        newTileAttrs.reserve(tileAttrs.size());
        for (const QJsonValue &attrData : tileAttrs) {
            auto attr = (uint16_t)attrData.toInt();
            auto attrTile = YUtils::getMapTileRecordDataFromShort(attr);
            newTileAttrs.push_back(attrTile);
        }
    } else {
        YUtils::printDebug("Could not load brush, missing 'tileAttrs'",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush, missing 'tileAttrs'");
        return false;
    }

    // Palette Offset
    uint8_t filePaletteOffsetValue = 0;
    if (const QJsonValue filePaletteOffset = json["paletteOffset"]; filePaletteOffset.isDouble()) {
        filePaletteOffsetValue = (uint8_t)filePaletteOffset.toInt();
    } else {
        YUtils::printDebug("'paletteOffset' not found",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush, missing 'paletteOffset'");
        return false;
    }

    // Saved width (int, stored as double)
    int storedWidth = 0;
    if (const QJsonValue storedWidthValue = json["width"]; storedWidthValue.isDouble()) {
        storedWidth = storedWidthValue.toInt();
    } else {
        YUtils::printDebug("Value 'width' not found",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush, missing 'width'");
        return false;
    }

    // Brush name
    std::string newBrushName = "error";
    if (const QJsonValue brushName = json["name"]; brushName.isString()) {
        auto loadedBrushName = brushName.toString().toStdString();
        newBrushName = loadedBrushName;
    } else {
        YUtils::printDebug("Could not load brush, missing 'brushTileset'",DebugType::ERROR);
        YUtils::popupAlert("Could not load brush, missing 'brushTileset'");
        return false;
    }

    // CHECK the palette offset
    uint8_t paletteOffset = (uint8_t)this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    if (paletteOffset != filePaletteOffsetValue) {
        std::stringstream ssPalOffMismatch;
        ssPalOffMismatch << "Mismatch: file paletteOffset vs current bg paletteOffset: 0x";
        ssPalOffMismatch << std::hex << (uint16_t)filePaletteOffsetValue << " vs 0x" << (uint16_t)paletteOffset;
        YUtils::printDebug(ssPalOffMismatch.str(),DebugType::ERROR);
        YUtils::popupAlert(ssPalOffMismatch.str());
        return false;
    }

    // Create the brush itself
    TileBrush newBrush;
    newBrush.brushTileset = newBrushTileset;
    newBrush.paletteOffset = filePaletteOffsetValue;
    newBrush.brushWidth = storedWidth;
    newBrush.tileAttrs = newTileAttrs;
    newBrush.name = newBrushName;
    
    // for (int y = 0; y < this->brushTable->rowCount(); y++) {
    //     for (int x = 0; x < this->brushTable->columnCount(); x++) {
    //         auto item = this->brushTable->item(y,x);
    //         uint32_t index = x + (y*this->brushTable->columnCount());
    //         auto mapTile = globalSettings.currentBrush->tileAttrs.at(index);
    //         if (item == nullptr) {
    //             item = new QTableWidgetItem();
    //             this->brushTable->setItem(y,x,item);
    //             item->setData(PixelDelegateData::DRAW_BG1,true);
    //             item->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
    //         }
    //         auto pal = mapTile.paletteId + paletteOffset;
    //         item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(mapTile.tileId).tiles);
    //         item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[pal]);
    //         item->setData(PixelDelegateData::TILE_ID_BG1,mapTile.tileId);
    //         item->setData(PixelDelegateData::PALETTE_ID_BG1,pal);
    //         item->setData(PixelDelegateData::FLIP_H_BG1,mapTile.flipH);
    //         item->setData(PixelDelegateData::FLIP_V_BG1,mapTile.flipV);
    //     }
    // }
    // this->brushTable->updateBrushDims();
    // if (globalSettings.brushW != storedWidth) {
    //     YUtils::printDebug("Mismatch in auto dimensions vs stored dimensions",DebugType::WARNING);
    //     // Continue, for now
    // }
    globalSettings.brushes.push_back(newBrush);
    this->updateStampList();
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
    uint8_t paletteOffset = (uint8_t)this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    // Fill it
    for (auto bit = globalSettings.brushes.begin(); bit != globalSettings.brushes.end(); bit++) {
        auto curBrush = *bit;
        auto newItem = new QListWidgetItem(this->stampList);
        auto brushImbz = curBrush.brushTileset;
        if (brushImbz.compare(currentScenImbz) != 0) {
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEnabled);
        }
        if (curBrush.paletteOffset != paletteOffset) {
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
    int paletteOffset = (int)this->yidsRom->chartileVramPaletteOffset[curBg->getInfo()->charBaseBlock];
    globalSettings.currentBrush->paletteOffset = paletteOffset;
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
    //YUtils::printDebug("stampListSelectedRowChanged");
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
    uint width = BrushTable::CELL_COUNT_DIMS; // temp, brushes should be size agnostic (TODO)
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
    int brushIndex = this->stampList->currentRow();
    if (brushIndex == -1) {
        YUtils::printDebug("No brush selected",DebugType::VERBOSE);
        return;
    }
    auto brushName = globalSettings.brushes.at(brushIndex).name;
    std::stringstream ss;
    ss << "Deleting brush '" << brushName << "'";
    YUtils::printDebug(ss.str(),DebugType::VERBOSE);
    globalSettings.brushes.erase(std::next(globalSettings.brushes.begin(),brushIndex));
    this->updateStampList();
}

void BrushWindow::loadBrushFile() {
    auto fileLoadPaths = QFileDialog::getOpenFileNames(this,tr("Open brush"),".",tr("YIDS Brush files (*.ydb)"));
    if (fileLoadPaths.isEmpty()) {
        YUtils::printDebug("Brush load canceled",DebugType::VERBOSE);
        return;
    }
    for (int i = 0; i < fileLoadPaths.size(); i++) {
        auto fileLoadPath = fileLoadPaths.at(i);
        if (!std::filesystem::exists(fileLoadPath.toStdString())) {
            YUtils::printDebug("Selected file does not exist",DebugType::ERROR);
            return;
        }
        this->loadBrushFileToList(fileLoadPath.toStdString());
    }
}

void BrushWindow::exportBrush() {
    this->saveCurrentBrushToFile();
}
