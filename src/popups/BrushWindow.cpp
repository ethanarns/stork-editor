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
    auto mainLayout = new QVBoxLayout(this);

    this->brushTable = new BrushTable(this,rom);
    mainLayout->addWidget(this->brushTable);
    
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
    auto brushName = new QLineEdit(this);
    brushName->setObjectName("textboxBrushName");
    brushName->setText("TODO");
    // ConnectMe
    bar3->addWidget(brushName);
    // Save brush
    auto saveBrush = new QPushButton("&Save YDB",this);
    saveBrush->setObjectName("buttonSaveBrushYdb");
    saveBrush->setDisabled(true);
    // ConnectMe
    bar3->addWidget(saveBrush);

    mainLayout->addLayout(bar1);
    mainLayout->addLayout(bar2);
    mainLayout->addLayout(bar3);

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
    std::map<uint32_t,Chartile> tilesMap = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground)->getVramChartiles();
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
            item->setData(PixelDelegateData::PIXEL_ARRAY_BG1,tilesMap.at(mapTile.tileId).tiles);
            item->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->backgroundPalettes[mapTile.paletteId]);
            item->setData(PixelDelegateData::TILE_ID_BG1,mapTile.tileId);
            item->setData(PixelDelegateData::PALETTE_ID_BG1,mapTile.paletteId);
            item->setData(PixelDelegateData::FLIP_H_BG1,mapTile.flipH);
            item->setData(PixelDelegateData::FLIP_V_BG1,mapTile.flipV);
        }
    }
    return true;
}
