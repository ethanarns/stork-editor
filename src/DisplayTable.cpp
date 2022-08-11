#include "DisplayTable.h"
#include "PixelDelegate.h"
#include "yidsrom.h"
#include "utils.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <iostream>
#include <sstream>

using namespace std;

DisplayTable::DisplayTable(QWidget* parent,YidsRom* rom) {
    Q_UNUSED(parent);
    this->shouldShowCollision = true;

    this->yidsRom = rom;

    // Layout //
    this->horizontalHeader()->setMinimumSectionSize(0);
    this->horizontalHeader()->setDefaultSectionSize(DisplayTable::CELL_SIZE_PX);
    this->verticalHeader()->setMinimumSectionSize(0);
    this->verticalHeader()->setDefaultSectionSize(DisplayTable::CELL_SIZE_PX);
    this->setRowCount(DisplayTable::CELL_COUNT_H);
    this->setColumnCount(DisplayTable::CELL_COUNT_W);
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->setShowGrid(false);
    this->setStyleSheet("QTableView::item {margin: 0;padding: 0;}");
    this->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable text editing

    // Drag and Drop //
    this->setMouseTracking(true);

    setItemDelegate(new PixelDelegate);

    QTableWidget::connect(this, &QTableWidget::cellEntered, this, &DisplayTable::cellEnteredTriggered);
    QTableWidget::connect(this, &QTableWidget::cellClicked, this, &DisplayTable::displayTableClicked);
}

/**
 * @brief Places a tile on the DisplayTable. Creates if it doesn't exist, updates
 * if it already exists.
 * 
 * @param x X Position of tile
 * @param y Y Position of tile
 * @param pren ChartilePreRenderData
 * @param whichBg which background to draw to
 */
void DisplayTable::putTileBg(uint32_t x, uint32_t y, ChartilePreRenderData &pren, uint16_t whichBg) {
    if (whichBg == 0 || whichBg > 3) {
        cerr << "[ERROR] Invalid BG put index " << whichBg << endl;
        exit(EXIT_FAILURE);
    }
    // NOTE: Remove me once BG3 is supported
    if (whichBg == 3) {
        cout << "[WARN] BG 3 is not yet supported" << endl;
        return;
    }
    if (x > (uint32_t)this->columnCount()) {
        std::cerr << "[ERROR] X value too high: " << hex << x << std::endl;
        return;
    }
    if (y > (uint32_t)this->rowCount()) {
        std::cerr << "[ERROR] Y value too high: " << hex << y << std::endl;
        return;
    }
    uint32_t pixelTileSize = 0;
    if (whichBg == 2) {
        pixelTileSize = this->yidsRom->pixelTilesBg2.size();
    } else if (whichBg == 1) {
        pixelTileSize = this->yidsRom->pixelTilesBg1.size();
    }
    if (pixelTileSize < 1) {
        std::cerr << "[ERROR] objsetPixelTiles are empty, cannot place tile" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (pren.tileId >= pixelTileSize) {
        std::cerr << "[ERROR] Tile ID '" << hex << pren.tileId;
        std::cerr << "' is greater than objsetPixelTiles count " << hex << pixelTileSize << std::endl;
        return;
    }
    int pal = (int)pren.paletteId; // int is more commonly used to access, so convert it early
    if (pal > 0xf) {
        cerr << "paletteId unusually high, got " << hex << pal << endl;
        pal = 0;
    }
    Chartile loadedTile;
    if (whichBg == 2) {
        loadedTile = this->yidsRom->pixelTilesBg2.at(pren.tileId);
    } else if (whichBg == 1) {
        loadedTile = this->yidsRom->pixelTilesBg1.at(pren.tileId);
    } else {
        cerr << "[ERROR] Unsupported BG: " << whichBg << endl;
        return;
    }
    auto potentialExisting = this->item(y,x);
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets make a new one and set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        if (whichBg == 2) {
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
            newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->currentPalettes[pal]);
            newItem->setData(PixelDelegateData::FLIP_H_BG2,pren.flipH);
            newItem->setData(PixelDelegateData::FLIP_V_BG2,pren.flipV);
            newItem->setData(PixelDelegateData::TILEATTR_BG2,(uint)pren.tileAttr);
        } else if (whichBg == 1) {
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
            newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->currentPalettes[pal]);
            newItem->setData(PixelDelegateData::FLIP_H_BG1,pren.flipH);
            newItem->setData(PixelDelegateData::FLIP_V_BG1,pren.flipV);
            newItem->setData(PixelDelegateData::TILEATTR_BG1,(uint)pren.tileAttr);
        }

        // Only doing collision here because there's no data for it, so create it
        newItem->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::CLEAR);
        newItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
        // Debug stuff
        newItem->setData(PixelDelegateData::DEBUG_DATA,loadedTile.index);
        this->setItem(y,x,newItem);
    } else {
        // There is already an item here, lets just update it
        if (whichBg == 2) {
            potentialExisting->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
            potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->currentPalettes[pal]);
            potentialExisting->setData(PixelDelegateData::FLIP_H_BG2,pren.flipH);
            potentialExisting->setData(PixelDelegateData::FLIP_V_BG2,pren.flipV);
            potentialExisting->setData(PixelDelegateData::TILEATTR_BG2,(uint)pren.tileAttr);
        } else if (whichBg == 1) {
            potentialExisting->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
            potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->currentPalettes[pal]);
            potentialExisting->setData(PixelDelegateData::FLIP_H_BG1,pren.flipH);
            potentialExisting->setData(PixelDelegateData::FLIP_V_BG1,pren.flipV);
            potentialExisting->setData(PixelDelegateData::TILEATTR_BG1,(uint)pren.tileAttr);
        }
        // Debug
        potentialExisting->setData(PixelDelegateData::DEBUG_DATA,loadedTile.index);
    }
}

void DisplayTable::cellEnteredTriggered(int y, int x) {
    //cout << "Cell entered at x,y: " << hex << x << "," << y << endl;
    QTableWidgetItem* curCell = this->item(y,x);
    if (curCell == nullptr) {
        return;
    } else {
        //cout << hex << curCell->data(PixelDelegateData::TILEATTR_BG2).toUInt() << endl;
    }
}

void DisplayTable::displayTableClicked(int row, int column) {
    cout << hex << row << "," << hex << column << endl;
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        // Nothing has loaded yet, cancel
        return;
    }
    cout << dec << curCell->data(PixelDelegateData::DEBUG_DATA).toInt() << endl;
}

void DisplayTable::setCellCollision(int row, int column, CollisionDraw colType) {
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        // Make a new one
        curCell = new QTableWidgetItem();
    }
    curCell->setData(PixelDelegateData::COLLISION_DRAW,colType);
}

void DisplayTable::initCellCollision() {
    if (this->yidsRom->canvasWidthCol < 1) {
        cerr << "[ERROR] Canvas Width for Collision is invalid! " << endl;
        return;
    }
    const uint32_t cutOff = this->yidsRom->canvasWidthCol/2;
    const int CELL_LIST_SIZE = this->yidsRom->collisionTileArray.size();
    if (CELL_LIST_SIZE < 1) {
        cerr << "[ERROR] Collision Tile Array is empty!" << endl;
        return;
    }
    for (int colIndex = 0; colIndex < CELL_LIST_SIZE; colIndex++) {
        const auto curCol = this->yidsRom->collisionTileArray.at(colIndex);
        uint32_t y = (colIndex / cutOff)*2;
        uint32_t x = (colIndex % cutOff)*2;
        if (curCol == CollisionType::SQUARE) {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
        } else if (curCol == CollisionType::PLATFORM_PASSABLE) {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::ZIG_ZAG);
            this->setCellCollision(y+1,x+1,CollisionDraw::ZIG_ZAG);
        } else if (curCol == CollisionType::DOWN_RIGHT_45) {
            this->setCellCollision(y,  x,  CollisionDraw::DIAG_DOWN_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT);
            this->setCellCollision(y+1,x+1,CollisionDraw::DIAG_DOWN_RIGHT);
        } else if (curCol == CollisionType::UP_RIGHT_45) {
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
            this->setCellCollision(y+1,x,  CollisionDraw::DIAG_UP_RIGHT);
            this->setCellCollision(y  ,x+1,CollisionDraw::DIAG_UP_RIGHT);
        } else if (curCol == CollisionType::STATIC_COIN) {
            // Don't draw, make a coin
            this->placeObjectTile(x+1,y,0,0,0,2,4,PaletteFileName::OBJSET);
        } else if (curCol != 0) { // Unknown, draw temp
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
        }
    }
}

void DisplayTable::toggleShowCollision() {
    if (this->shouldShowCollision == true) {
        this->shouldShowCollision = false;
        auto allItems = this->items(nullptr);
    } else {
        this->shouldShowCollision = true;
    }
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                potentialItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
            }
        }
    }
}

void DisplayTable::updateBg() {
    /******************
     ** BACKGROUND 2 **
     ******************/
    uint32_t preRenderSizeBg2 = this->yidsRom->preRenderDataBg2.size();
    if (preRenderSizeBg2 == 0) {
        std::cerr << "[WARN] preRenderDataBg2 is empty" << std::endl;
        return;
    }
    if (this->yidsRom->canvasWidthBg2 == 0) {
        std::cerr << "[ERROR] Canvas Width for BG2 was never set!" << std::endl;
        return;
    }
    if (this->yidsRom->pixelTilesBg2.size() < 1) {
        std::cerr << "[WARN] Cannot update BG2, missing pixelTilesBg2" << std::endl;
        return;
    }
    //const uint32_t cutOffBg2 = 0x10*32.5;
    const uint32_t cutOffBg2 = this->yidsRom->canvasWidthBg2;
    for (uint32_t preRenderIndex = 0; preRenderIndex < preRenderSizeBg2; preRenderIndex++) {
        uint32_t y = preRenderIndex / cutOffBg2;
        uint32_t x = preRenderIndex % cutOffBg2;
        ChartilePreRenderData curShort = YUtils::getCharPreRender(this->yidsRom->preRenderDataBg2.at(preRenderIndex));
        //cout << "x: " << hex << x << ", y: " << hex << y << endl;
        this->putTileBg(x,y,curShort,2);
    }
    /******************
     ** BACKGROUND 1 **
     ******************/
    uint32_t preRenderSizeBg1 = this->yidsRom->preRenderDataBg1.size();
    if (preRenderSizeBg1 == 0) {
        std::cerr << "[WARN] preRenderDataBg1 is empty" << std::endl;
        return;
    }
    if (this->yidsRom->canvasWidthBg1 == 0) {
        std::cerr << "[ERROR] Canvas Width for BG1 was never set!" << std::endl;
        return;
    }
    if (this->yidsRom->pixelTilesBg1.size() < 1) {
        std::cerr << "[WARN] Cannot update BG1, missing pixelTilesBg1" << std::endl;
        return;
    }
    const uint32_t cutOffBg1 = this->yidsRom->canvasWidthBg1;
    for (uint32_t preRenderIndex1 = 0; preRenderIndex1 < preRenderSizeBg1; preRenderIndex1++) {
        uint32_t y = preRenderIndex1 / cutOffBg1;
        uint32_t x = preRenderIndex1 % cutOffBg1;
        ChartilePreRenderData curShort = YUtils::getCharPreRender(this->yidsRom->preRenderDataBg1.at(preRenderIndex1));
        this->putTileBg(x,y,curShort,1);
    }
}

void DisplayTable::updateObjects() {
    if (this->yidsRom->loadedLevelObjects.size() == 0) {
        cout << "[WARN] No objects loaded" << endl;
        return;
    }
    for (auto it = this->yidsRom->loadedLevelObjects.cbegin(); it != this->yidsRom->loadedLevelObjects.cend(); it++) {
        auto x = it->xPosition;
        auto y = it->yPosition;
        auto potentialExisting = this->item(y,x);
        // Very basic pointing to position
        if (potentialExisting == nullptr) {
            cout << "[WARN] No tile to place object on at x: " << hex << x << ", y: " << hex << y << endl; 
            potentialExisting = new QTableWidgetItem(); // Make it NOT null lmao
        }
        auto objectGraphicsMeta = LevelObject::getObjectGraphicMetadata(*it);
        if (objectGraphicsMeta.tilesCount == 0) {
            potentialExisting->setData(PixelDelegateData::OBJECT_ID,(int)it->objectId);
            std::stringstream ss;
            ss << hex << (int)it->objectId;
            ss << endl;
            ss << "Object description goes here";
            potentialExisting->setToolTip(tr(ss.str().c_str()));
        } else {
            this->placeObjectTile(
                (uint32_t)x,(uint32_t)y,
                objectGraphicsMeta.tilesSectorOffset,
                objectGraphicsMeta.subTile,
                objectGraphicsMeta.paletteSectorOffset,
                objectGraphicsMeta.tileWidth,
                objectGraphicsMeta.tilesCount,
                objectGraphicsMeta.whichPaletteFile
            );
        }
    }
}

int DisplayTable::wipeTable() {
    uint32_t xWidth = this->columnCount();
    uint32_t yHeight = this->rowCount();
    for (uint32_t col = 0; col < xWidth; col++) {
        for (uint32_t row = 0; row < yHeight; row++) {
            auto currentItem = this->item(row,col);
            if (currentItem != nullptr) {
                this->takeItem(row,col);
                delete currentItem;
            }
        }
    }
    return 0;
}

void DisplayTable::placeObjectTile(
    uint32_t x, uint32_t y,
    uint32_t objectOffset,
    uint32_t subTile,
    uint32_t paletteOffset,
    uint32_t spriteWidth,
    uint32_t tilesLength,
    PaletteFileName paletteFile
) {
    auto objectPalette = this->yidsRom->currentPalettes[0]; // Default
    if (paletteOffset != 0) {
        if (paletteFile == PaletteFileName::OBJSET) {
            objectPalette = this->yidsRom->objsetPalettes[paletteOffset].paletteData;
        } else if (paletteFile == PaletteFileName::OBJEFFECT) {
            objectPalette = this->yidsRom->effectPalettes[paletteOffset].paletteData;
        } else {
            std::cerr << "[ERROR] Unknown PaletteFileName enum value: " << hex << paletteFile << endl;
        }
    }

    auto objectVector = this->yidsRom->objsetPixelTiles[objectOffset];
    uint32_t subLength = objectVector.size();
    uint32_t subIndex = 0x00;

    bool endLoop1 = false;
    uint32_t loopIndex = 0;
    while (endLoop1 == false) {
        // Frame record //
        uint16_t posRecordOffset = YUtils::getUint16FromVec(objectVector,subIndex+0);
        if (posRecordOffset == 0) {
            endLoop1 = true;
            break;
        }

        if (subTile == loopIndex) {
            // 3rd byte
            uint8_t animationHoldTime = objectVector.at(subIndex+2);
            Q_UNUSED(animationHoldTime);
            // The first number is an offset to the position record
            // It is offset FROM the current record
            uint16_t addrOfPositionRecord = subIndex + posRecordOffset;

            // Position record //
            uint16_t frameIndex = YUtils::getUint16FromVec(objectVector,addrOfPositionRecord);
            uint32_t tileStart = frameIndex << 4;
            int16_t xOffset = YUtils::getInt16FromVec(objectVector, addrOfPositionRecord + 2); // Needs printf to show up:
            int16_t yOffset = YUtils::getInt16FromVec(objectVector, addrOfPositionRecord + 4); // printf("y: %d\n",yOffset);
            const int16_t singleTileDim = static_cast<int16_t>(Constants::SINGLE_TILE_DIM);
            xOffset = YUtils::roundI16Down(xOffset,singleTileDim);
            yOffset = YUtils::roundI16Down(yOffset,singleTileDim);
            //printf("x: %d, y: %d\n",xOffset, yOffset);

            // uint16_t constructionCode = YUtils::getUint16FromVec(objectVector,addrOfPositionRecord + 6);
            // uint32_t tileStart = frameIndex << 4;
            // uint16_t constructionOffset = constructionCode & 0x1f; // 01FFA6E0
            // constructionOffset = (constructionOffset << 1) + constructionOffset; // 01FFA6E4
            
            uint32_t subEnd = tileStart + Constants::CHARTILE_DATA_SIZE;
            if (tileStart+Constants::CHARTILE_DATA_SIZE > subLength) {
                cerr << "[WARN] Tried to get too big a chunk! Wanted " << hex << subEnd;
                cerr << ", only had " << hex << subLength << endl;
            } else {
                uint32_t tileOffsetIndex = 0;
                const int16_t baseX = x + (xOffset/8);
                const int16_t baseY = y + (yOffset/8);
                while (tileOffsetIndex < tilesLength) {
                    uint32_t curX = (tileOffsetIndex % spriteWidth);
                    uint32_t curY = (tileOffsetIndex / spriteWidth);
                    uint32_t curTileAddressOffset = tileOffsetIndex * Constants::CHARTILE_DATA_SIZE;
                    auto curTiles = YUtils::subVector(objectVector,
                        tileStart + curTileAddressOffset,
                        subEnd + curTileAddressOffset
                    );
                    auto tileItem = this->item(baseY+curY,baseX+curX);
                    if (tileItem == nullptr) {
                        tileItem = new QTableWidgetItem();
                    }
                    tileItem->setData(PixelDelegateData::OBJECT_TILES,YUtils::tileVectorToQByteArray(curTiles));
                    tileItem->setData(PixelDelegateData::OBJECT_PALETTE,objectPalette);
                    tileItem->setData(PixelDelegateData::FLIP_H_BG2,0);
                    tileItem->setData(PixelDelegateData::FLIP_V_BG2,0);

                    tileOffsetIndex++;
                }
                return; // Done
            } 
        } else {
            loopIndex++;
            subIndex += 4;
        }
    }
}