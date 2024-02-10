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
        std::stringstream ssBadBg;
        ssBadBg << "Invalid BG put index: " << whichBg;
        YUtils::printDebug(ssBadBg.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    // NOTE: Remove me once BG3 is supported
    if (whichBg == 3) {
        YUtils::printDebug("BG 3 tile placement is not yet supported",DebugType::WARNING);
        return;
    }
    if (x > (uint32_t)this->columnCount()) {
        std::stringstream ssXvaluHigh;
        ssXvaluHigh << "X value too high: " << hex << x;
        YUtils::printDebug(ssXvaluHigh.str(),DebugType::ERROR);
        return;
    }
    if (y > (uint32_t)this->rowCount()) {
        std::stringstream ssYvaluHigh;
        ssYvaluHigh << "Y value too high: " << hex << y;
        YUtils::printDebug(ssYvaluHigh.str(),DebugType::ERROR);
        return;
    }
    auto pal = pren.paletteId;
    Chartile loadedTile;
    if (whichBg == 2) {
        try {
            loadedTile = this->yidsRom->pixelTilesBg2.at(pren.tileId);
        } catch (...) {
            // 0 often just means "empty," but is not consistent
            // Use this as a fallback until you find out
            if (pren.tileId != 0) {
                std::stringstream ssTile;
                ssTile << "Could not get certain tileId for BG2: " << std::hex << pren.tileId;
                YUtils::printDebug(ssTile.str(),DebugType::ERROR);
            }
            return;
        }
        
        pal += this->yidsRom->paletteOffsetBg2;
    } else if (whichBg == 1) {
        try {
            loadedTile = this->yidsRom->pixelTilesBg1.at(pren.tileId);
        } catch (...) {
            if (pren.tileId != 0) {
                std::stringstream ssTile;
                ssTile << "Could not get certain tileId for BG1: " << std::hex << pren.tileId;
                YUtils::printDebug(ssTile.str(),DebugType::ERROR);
            }
            return;
        }

        pal += this->yidsRom->paletteOffsetBg1;
    } else {
        std::stringstream ssbg;
        ssbg << "Unsupported BG: " << hex << whichBg;
        YUtils::printDebug(ssbg.str(),DebugType::ERROR);
        return;
    }
    auto potentialExisting = this->item(y,x);
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets make a new one and set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        if (whichBg == 2) {
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
            if (this->yidsRom->colorModeBg2 == BgColorMode::MODE_16) {
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->currentPalettes[pal]);
            } else {
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(this->yidsRom->paletteOffsetBg2 + 1));
            }
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
            if (this->yidsRom->colorModeBg2 == BgColorMode::MODE_16) {
                potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->currentPalettes[pal]);
            } else {
                potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(this->yidsRom->paletteOffsetBg2 + 1));
            }
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
    QTableWidgetItem* curCell = this->item(y,x);
    if (curCell == nullptr) {
        return;
    } else {
        if (curCell->isSelected()) {
            this->setCursor(Qt::OpenHandCursor);
        } else {
            this->setCursor(Qt::ArrowCursor);
        }
    }
}

void DisplayTable::displayTableClicked(int row, int column) {
    cout << hex << row << "," << hex << column << endl;
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        // Nothing has loaded yet, cancel
        return;
    }
    if (!curCell->data(PixelDelegateData::OBJECT_UUID).isNull()) {
        cout << dec << curCell->data(PixelDelegateData::OBJECT_UUID).toInt() << endl;
    }
    if (!curCell->data(PixelDelegateData::PIXEL_ARRAY_BG2).isNull()) {
        auto pixArray2 = curCell->data(PixelDelegateData::PIXEL_ARRAY_BG2).toByteArray();
        YUtils::printDebug("Pixel Array for BG 2:",DebugType::VERBOSE);
        YUtils::printQbyte(pixArray2);
    }
    if (!curCell->data(PixelDelegateData::PIXEL_ARRAY_BG1).isNull()) {
        auto pixArray1 = curCell->data(PixelDelegateData::PIXEL_ARRAY_BG1).toByteArray();
        YUtils::printDebug("Pixel Array for BG 1:",DebugType::VERBOSE);
        YUtils::printQbyte(pixArray1);
    }

    if (this->layerSelectMode == LayerSelectMode::SPRITES_LAYER) {
        if (!curCell->data(PixelDelegateData::OBJECT_UUID).isNull()) {
            this->clearSelection();
            this->selectedObjects.clear();
            uint32_t curUuid = curCell->data(PixelDelegateData::OBJECT_UUID).toUInt();
            std::stringstream ssSprite;
            ssSprite << "Selecting cell with Object UUID " << std::hex << curUuid;
            YUtils::printDebug(ssSprite.str(),DebugType::VERBOSE);
            this->selectItemByUuid(curUuid);
        } else {
            this->clearSelection();
            this->selectedObjects.clear();
        }
    }
}

void DisplayTable::selectItemByUuid(uint32_t uuid) {
    if (this->layerSelectMode != LayerSelectMode::SPRITES_LAYER) {
        YUtils::printDebug("Items should not be selected when not in SPRITES_LAYER mode",DebugType::ERROR);
        return;
    }
    this->selectedObjects.push_back(uuid);
    auto allItems = this->findItems("sprite",Qt::MatchExactly);
    if (allItems.size() == 0) {
        YUtils::printDebug("Zero sprites given text data!",DebugType::WARNING);
        return;
    }
    for (int i = 0; i < allItems.size(); i++) {
        auto potentialItem = allItems.at(i);
        if (potentialItem != nullptr) {
            uint32_t foundUuid = potentialItem->data(PixelDelegateData::OBJECT_UUID).toUInt();
            if (foundUuid == uuid) {
                this->setItemSelected(potentialItem,true);
            }
        }
    }
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
        YUtils::printDebug("Canvas Width for Collision is invalid!",DebugType::ERROR);
        return;
    }
    const uint32_t cutOff = this->yidsRom->canvasWidthCol/2;
    const int CELL_LIST_SIZE = this->yidsRom->collisionTileArray.size();
    if (CELL_LIST_SIZE < 1) {
        YUtils::printDebug("Collision Tile Array is empty!",DebugType::ERROR);
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
            this->placeObjectTile(x+1,y,0,0,0,2,4,ObjectFileName::OBJSET,ObjectFileName::OBJSET,0,0,this->yidsRom->levelObjectLoadIndex++);
        } else if (curCol != 0) { // Unknown, draw temp
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT);
        }
    }
}

void DisplayTable::updateShowCollision() {
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

void DisplayTable::moveSprite(int uuid, int xOffset, int yOffset) {
    this->wipeObject(uuid);
    this->yidsRom->moveObject(uuid,xOffset,yOffset);
}

void DisplayTable::moveCurrentlySelectedSprites(int xOffset, int yOffset) {
    for (auto it = this->selectedObjects.begin(); it != this->selectedObjects.end(); it++) {
        this->moveSprite(*it, xOffset, yOffset);
    }
    this->updateObjects();
}

void DisplayTable::setLayerDraw(int whichLayer, bool shouldDraw) {
    if (whichLayer == 1) {
        this->drawBg1 = shouldDraw;
    } else if (whichLayer == 2) {
        this->drawBg2 = shouldDraw;
    } else if (whichLayer == 3) {
        this->drawBg3 = shouldDraw;
    } else {
        this->drawObjects = shouldDraw;
    }
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                if (whichLayer == 1) {
                    potentialItem->setData(PixelDelegateData::DRAW_BG1,this->drawBg1);
                } else if (whichLayer == 2) {
                    potentialItem->setData(PixelDelegateData::DRAW_BG2,this->drawBg2);
                } else if (whichLayer == 3) {
                    potentialItem->setData(PixelDelegateData::DRAW_BG3,this->drawBg3);
                } else {
                    potentialItem->setData(PixelDelegateData::DRAW_OBJECTS,this->drawObjects);
                }
            }
        }
    }
}

void DisplayTable::updateBg() {
    auto newCanvasHeight = this->yidsRom->getGreatestCanvasHeight();
    if (newCanvasHeight == 0) {
        YUtils::printDebug("Canvas height calculation failed!",DebugType::ERROR);
    } else {
        this->setRowCount(newCanvasHeight);
    }
    
    auto newCanvasWidth = this->yidsRom->getGreatestCanvasWidth();
    if (newCanvasWidth == 0) {
        YUtils::printDebug("Canvas width calculation failed!",DebugType::ERROR);
    } else {
        this->setColumnCount(newCanvasWidth);
    }

    /******************
     ** BACKGROUND 2 **
     ******************/
    uint32_t preRenderSizeBg2 = this->yidsRom->preRenderDataBg2.size();
    if (preRenderSizeBg2 == 0) {
        YUtils::printDebug("preRenderDataBg2 is empty",DebugType::WARNING);
    }
    if (this->yidsRom->canvasWidthBg2 == 0) {
        YUtils::printDebug("Canvas Width for BG2 was never set!",DebugType::ERROR);
        return;
    }
    if (this->yidsRom->pixelTilesBg2.size() < 1) {
        YUtils::printDebug("Cannot update BG2, missing pixelTilesBg2",DebugType::WARNING);
    }

    const uint32_t cutOffBg2 = this->yidsRom->canvasWidthBg2;
    uint32_t bg2LeftOffset = 0;
    while (bg2LeftOffset < newCanvasWidth) {
        for (uint32_t preRenderIndex = 0; preRenderIndex < preRenderSizeBg2; preRenderIndex++) {
            uint32_t y = preRenderIndex / cutOffBg2;
            uint32_t x = (preRenderIndex % cutOffBg2) + bg2LeftOffset;
            // Note: You might need to apply this to other layers later
            // -1 because if you go the full length, it loops back around. Remember, .length - 1!
            if (x > newCanvasWidth-1) {
                continue;
            }
            ChartilePreRenderData curShort = YUtils::getCharPreRender(this->yidsRom->preRenderDataBg2.at(preRenderIndex),this->yidsRom->colorModeBg2);
            this->putTileBg(x,y,curShort,2);
        }
        bg2LeftOffset += cutOffBg2;
    }

    /******************
     ** BACKGROUND 1 **
     ******************/
    uint32_t preRenderSizeBg1 = this->yidsRom->preRenderDataBg1.size();
    if (preRenderSizeBg1 == 0) {
        YUtils::printDebug("preRenderDataBg1 is empty",DebugType::WARNING);
        return;
    }
    if (this->yidsRom->canvasWidthBg1 == 0) {
        YUtils::printDebug("Canvas Width for BG1 was never set!",DebugType::ERROR);
        return;
    }
    if (this->yidsRom->pixelTilesBg1.size() < 1) {
        YUtils::printDebug("Cannot update BG1, missing pixelTilesBg1",DebugType::WARNING);
        return;
    }
    const uint32_t cutOffBg1 = this->yidsRom->canvasWidthBg1;
    for (uint32_t preRenderIndex1 = 0; preRenderIndex1 < preRenderSizeBg1; preRenderIndex1++) {
        uint32_t y = preRenderIndex1 / cutOffBg1;
        uint32_t x = preRenderIndex1 % cutOffBg1;
        ChartilePreRenderData curShort = YUtils::getCharPreRender(this->yidsRom->preRenderDataBg1.at(preRenderIndex1),this->yidsRom->colorModeBg1);
        this->putTileBg(x,y,curShort,1);
    }
}

void DisplayTable::updateObjects() {
    if (this->yidsRom->loadedLevelObjects.size() == 0) {
        YUtils::printDebug("No objects loaded, cannot update",DebugType::ERROR);
        return;
    }
    for (auto it = this->yidsRom->loadedLevelObjects.cbegin(); it != this->yidsRom->loadedLevelObjects.cend(); it++) {
        auto x = it->xPosition;
        auto y = it->yPosition;
        auto potentialExisting = this->item(y,x);
        if (potentialExisting == nullptr) {
            potentialExisting = new QTableWidgetItem();
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
                objectGraphicsMeta.whichPaletteFile,
                objectGraphicsMeta.whichObjectFile,
                objectGraphicsMeta.xPixelOffset,
                objectGraphicsMeta.yPixelOffset,
                it->uuid
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
    ObjectFileName paletteFile,
    ObjectFileName objectFile,
    uint32_t xPixelOffset,
    uint32_t yPixelOffset,
    uint32_t uuid
) {
    auto objectPalette = this->yidsRom->currentPalettes[0]; // Default
    if (paletteFile == ObjectFileName::OBJSET) {
        objectPalette = this->yidsRom->objsetPalettes[paletteOffset].paletteData;
    } else if (paletteFile == ObjectFileName::OBJEFFECT) {
        objectPalette = this->yidsRom->effectPalettes[paletteOffset].paletteData;
    } else {
        objectPalette = this->yidsRom->objectFiles[objectFile].objectPalettes[paletteOffset].paletteData;
    }

    std::vector<uint8_t> objectVector;
    if (objectFile == ObjectFileName::OBJSET) {
        objectVector = this->yidsRom->objsetPixelTiles[objectOffset];
    } else {
        objectVector = this->yidsRom->objectFiles[objectFile].objectPixelTiles[objectOffset];
    }
    
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
                std::stringstream ssChunk;
                ssChunk << "Tried to get too big a chunk! Wanted " << hex << subEnd;
                ssChunk << ", only had " << hex << subLength;
                YUtils::printDebug(ssChunk.str(),DebugType::WARNING);
            } else {
                uint32_t tileOffsetIndex = 0;
                const int16_t baseX = x + ((xOffset + xPixelOffset)/8);
                const int16_t baseY = y + ((yOffset + yPixelOffset)/8);
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
                    tileItem->setData(PixelDelegateData::OBJECT_UUID,uuid);
                    tileItem->setText("sprite");

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

void DisplayTable::wipeObject(int uuid) {
    auto allItems = this->findItems("sprite",Qt::MatchExactly);
    if (allItems.size() == 0) {
        YUtils::printDebug("Zero sprites given text data!",DebugType::WARNING);
        return;
    }
    for (int i = 0; i < allItems.size(); i++) {
        auto potentialItem = allItems.at(i);
        if (potentialItem != nullptr) {
            auto foundUuid = potentialItem->data(PixelDelegateData::OBJECT_UUID).toInt();
            if (foundUuid == uuid) {
                potentialItem->setData(PixelDelegateData::OBJECT_TILES,QByteArray());
            }
        }
    }
}