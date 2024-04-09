#include "DisplayTable.h"
#include "PixelDelegate.h"
#include "PixelDelegateEnums.h"
#include "yidsrom.h"
#include "utils.h"
#include "popups/BrushTable.h"
#include "StateCommands.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QDrag>
#include <QApplication>
#include <QRubberBand>

#include <iostream>
#include <sstream>
#include <cmath>

DisplayTable::DisplayTable(QWidget* parent,YidsRom* rom) {
    this->setParent(parent);
    this->shouldShowCollision = true;
    this->shouldDrawEntrances = true;
    this->shouldDrawExits = true;
    this->firstLayerDrawDone = false;

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
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::DragDrop);

    setItemDelegate(new PixelDelegate);

    this->selectorBand = new QRubberBand(QRubberBand::Rectangle,this);
    this->selectorBand->hide();
    this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

    QTableWidget::connect(this, &QTableWidget::cellEntered, this, &DisplayTable::cellEnteredTriggered);
    //QTableWidget::connect(this, &QTableWidget::cellClicked, this, &DisplayTable::displayTableClicked);
}

/**
 * @brief Places a tile on the DisplayTable. Primarily used for init
 * 
 * @param x X Position of tile
 * @param y Y Position of tile
 * @param pren MapTileRecordData
 * @param whichBg which background to draw to
 */
void DisplayTable::putTileBg(uint32_t x, uint32_t y, MapTileRecordData &pren, uint16_t whichBg) {
    if (whichBg == 0 || whichBg > 3) {
        std::stringstream ssBadBg;
        ssBadBg << "Invalid BG put index: " << whichBg;
        YUtils::printDebug(ssBadBg.str(),DebugType::FATAL);
        YUtils::popupAlert(ssBadBg.str());
        exit(EXIT_FAILURE);
    }
    if (x > (uint32_t)this->columnCount()) {
        std::stringstream ssXvaluHigh;
        ssXvaluHigh << "X value too high: " << std::hex << x;
        YUtils::printDebug(ssXvaluHigh.str(),DebugType::ERROR);
        return;
    }
    if (y > (uint32_t)this->rowCount()) {
        std::stringstream ssYvaluHigh;
        ssYvaluHigh << "Y value too high: " << std::hex << y;
        YUtils::printDebug(ssYvaluHigh.str(),DebugType::ERROR);
        return;
    }
    auto pal = pren.paletteId;
    Chartile loadedTile;
    auto scen = this->yidsRom->mapData->getScenByBg(whichBg,false);
    if (scen == nullptr) {
        YUtils::printDebug("SCEN for this BG is nullptr, skipping",DebugType::WARNING);
        return;
    }
    auto vramChartiles = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    try {
        loadedTile = vramChartiles.at(pren.tileId);
    } catch (...) {
        // 0 often just means "empty," but is not consistent
        // Use this as a fallback until you find out
        if (pren.tileId != 0) {
            std::stringstream ssTile;
            ssTile << "Could not get certain tileId for BG" << whichBg << ": " << std::hex << pren.tileId;
            YUtils::printDebug(ssTile.str(),DebugType::ERROR);
        }
        return;
    }

    pal += (uint8_t)this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    //pal += scen->paletteStartOffset - 1; // Bad, keep for info though

    auto bgItem = this->item(y,x);
    auto colorMode = scen->getInfo()->colorMode;
    QByteArray layerDrawOrder = this->yidsRom->mapData->getLayerOrder();
    if (layerDrawOrder.size() > 3) {
        YUtils::printDebug("Size error in layer order, should be 3, was:",DebugType::FATAL);
        YUtils::printQbyte(layerDrawOrder);
        exit(EXIT_FAILURE);
    }
    if (bgItem == nullptr) {
        bgItem = new QTableWidgetItem();
        this->setItem(y,x,bgItem);
    }
    if (whichBg == 2) {
        // BG 2 //
        bgItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
        if (colorMode == BgColorMode::MODE_16 || colorMode == BgColorMode::MODE_UNKNOWN) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else if (colorMode == BgColorMode::MODE_256) {
            // Note: the 256 palettes thing does not always start at 0x10 (including the +1)
            // 1-3, there's a palette missing from the palette screen that made this start at 0xf
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,scen->getPalette()->extendedPalette);
        } else {
            YUtils::printDebug("Unhandled color mode",DebugType::WARNING);
        }
        bgItem->setData(PixelDelegateData::FLIP_H_BG2,pren.flipH);
        bgItem->setData(PixelDelegateData::FLIP_V_BG2,pren.flipV);
        bgItem->setData(PixelDelegateData::TILEATTR_BG2,(uint)pren.tileAttr);
        bgItem->setData(PixelDelegateData::TILE_ID_BG2,(uint)pren.tileId);
    } else if (whichBg == 1) {
        // BG 1 //
        bgItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
        if (colorMode == BgColorMode::MODE_16 || colorMode == BgColorMode::MODE_UNKNOWN) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else if (colorMode == BgColorMode::MODE_256) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,scen->getPalette()->extendedPalette);
        } else {
            YUtils::printDebug("Unhandled color mode",DebugType::WARNING);
        }
        bgItem->setData(PixelDelegateData::FLIP_H_BG1,pren.flipH);
        bgItem->setData(PixelDelegateData::FLIP_V_BG1,pren.flipV);
        bgItem->setData(PixelDelegateData::TILEATTR_BG1,(uint)pren.tileAttr);
        bgItem->setData(PixelDelegateData::TILE_ID_BG1,(uint)pren.tileId);
    } else if (whichBg == 3) {
        // BG 3 //
        bgItem->setData(PixelDelegateData::PIXEL_ARRAY_BG3,loadedTile.tiles);
        if (colorMode == BgColorMode::MODE_16 || colorMode == BgColorMode::MODE_UNKNOWN) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else if (colorMode == BgColorMode::MODE_256) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,scen->getPalette()->extendedPalette);
        } else {
            YUtils::printDebug("Unhandled color mode",DebugType::WARNING);
        }
        bgItem->setData(PixelDelegateData::FLIP_H_BG3,pren.flipH);
        bgItem->setData(PixelDelegateData::FLIP_V_BG3,pren.flipV);
        bgItem->setData(PixelDelegateData::TILEATTR_BG3,(uint)pren.tileAttr);
        bgItem->setData(PixelDelegateData::TILE_ID_BG3,(uint)pren.tileId);
    }
    // Things to do for every layer:
    bgItem->setData(PixelDelegateData::LAYER_DRAW_ORDER,layerDrawOrder);
    bgItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
    bgItem->setData(PixelDelegateData::HOVER_TYPE,HoverType::NO_HOVER);
    bgItem->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::CLEAR);
    bgItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);
}

void DisplayTable::cellEnteredTriggered(int y, int x) {
    QTableWidgetItem* curCell = this->item(y,x);
    if (curCell == nullptr) {
        return;
    } else {
        // BG Brush hovering
        if (
            globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
            globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
            globalSettings.layerSelectMode == LayerMode::BG3_LAYER
        ) {
            // Round down
            int roundDownRowY = y;
            if (roundDownRowY % 2 != 0) { // Odd
                roundDownRowY--;
            }
            int roundDownColX = x;
            if  (roundDownColX % 2 != 0) { // Odd
                roundDownColX--;
            }
            // Clear surroundings
            this->updateSurrounding(y,x,100);
            curCell->setData(PixelDelegateData::HOVER_TYPE,HoverType::HOVER_SQUARE);
 
            // TODO: edges?

            this->setHover(roundDownRowY                        ,roundDownColX+globalSettings.brushW-1,HoverType::HOVER_TR);
            this->setHover(roundDownRowY+globalSettings.brushH-1,roundDownColX+globalSettings.brushW-1,HoverType::HOVER_BR);
            this->setHover(roundDownRowY+globalSettings.brushH-1,roundDownColX                        ,HoverType::HOVER_BL);
        } else if (globalSettings.layerSelectMode == LayerMode::COLLISION_LAYER) {
            this->updateSurrounding(y,x,100);
            // Round down
            int roundDownRowY = y;
            if (roundDownRowY % 2 != 0) { // Odd
                roundDownRowY--;
            }
            int roundDownColX = x;
            if  (roundDownColX % 2 != 0) { // Odd
                roundDownColX--;
            }
            this->setHover(roundDownRowY+0,roundDownColX+0,HoverType::HOVER_TL);
            this->setHover(roundDownRowY+1,roundDownColX+0,HoverType::HOVER_BL);
            this->setHover(roundDownRowY+0,roundDownColX+1,HoverType::HOVER_TR);
            this->setHover(roundDownRowY+1,roundDownColX+1,HoverType::HOVER_BR);
        } else if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
            if (curCell->data(PixelDelegateData::SPRITE_SELECTED).toBool() == true) {
                this->setCursor(Qt::OpenHandCursor);
            } else {
                this->setCursor(Qt::CrossCursor);
            }
        }
    }
}

void DisplayTable::printCellDebug(QTableWidgetItem *item, uint whichBg) {
    std::cout << "** Printing cell debug **" << std::endl;
    if (item == nullptr) {
        YUtils::printDebug("Item is null, can't print debug",DebugType::WARNING);
        return;
    }
    if (whichBg == 1) {
        if (!item->data(PixelDelegateData::PIXEL_ARRAY_BG1).isNull()) {
            auto tileAttr = item->data(PixelDelegateData::TILEATTR_BG1).toUInt();
            YUtils::printDebug(YUtils::getMapTileRecordDataFromShort(tileAttr).toString());
            auto pixArray2 = item->data(PixelDelegateData::PIXEL_ARRAY_BG1).toByteArray();
            YUtils::printDebug("Pixel Array for BG 1:",DebugType::VERBOSE);
            YUtils::printQbyte(pixArray2);
            YUtils::printDebug("Palette for BG 1:",DebugType::VERBOSE);
            auto pal = item->data(PixelDelegateData::PALETTE_ARRAY_BG1).toByteArray();
            YUtils::printQbyte(pal);
        }
    } else if (whichBg == 2) {
        if (!item->data(PixelDelegateData::PIXEL_ARRAY_BG2).isNull()) {
            auto tileAttr = item->data(PixelDelegateData::TILEATTR_BG2).toUInt();
            YUtils::printDebug(YUtils::getMapTileRecordDataFromShort(tileAttr).toString());
            auto pixArray2 = item->data(PixelDelegateData::PIXEL_ARRAY_BG2).toByteArray();
            YUtils::printDebug("Pixel Array for BG 2:",DebugType::VERBOSE);
            YUtils::printQbyte(pixArray2);
            YUtils::printDebug("Palette for BG2:",DebugType::VERBOSE);
            auto pal = item->data(PixelDelegateData::PALETTE_ARRAY_BG2).toByteArray();
            YUtils::printQbyte(pal);
        }
    } else if (whichBg == 3) {
        if (!item->data(PixelDelegateData::PIXEL_ARRAY_BG3).isNull()) {
            auto tileAttr = item->data(PixelDelegateData::TILEATTR_BG3).toUInt();
            YUtils::printDebug(YUtils::getMapTileRecordDataFromShort(tileAttr).toString());
            auto pixArray2 = item->data(PixelDelegateData::PIXEL_ARRAY_BG3).toByteArray();
            YUtils::printDebug("Pixel Array for BG 3:",DebugType::VERBOSE);
            YUtils::printQbyte(pixArray2);
            YUtils::printDebug("Palette for BG 3:",DebugType::VERBOSE);
            auto pal = item->data(PixelDelegateData::PALETTE_ARRAY_BG3).toByteArray();
            YUtils::printQbyte(pal);
        }
    } else {
        YUtils::printDebug("Unknown BG to debug",DebugType::ERROR);
    }
}

QPoint DisplayTable::getTopLeftOfSprite(uint32_t levelObjectUuid) {
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr && !potentialItem->data(PixelDelegateData::OBJECT_UUID).isNull()) {
                if (potentialItem->data(PixelDelegateData::OBJECT_UUID).toUInt() == levelObjectUuid) {
                    QPoint point(col,row);
                    return point;
                }
            }
        }
    }
    YUtils::printDebug("Could not find sprite with that UUID",DebugType::ERROR);
    return QPoint();
}

void DisplayTable::updateSurrounding(int row, int column, int distance) {
    int leftEnd = column - distance;
    if (leftEnd < 0) {
        leftEnd = 0;
    }
    int rightEnd = column + distance;

    int topEnd = row - distance;
    if (topEnd < 0) {
        topEnd = 0;
    }
    int bottomEnd = row + distance;

    for (int colIndex = leftEnd; colIndex < rightEnd; colIndex++) {
        for (int rowIndex = topEnd; rowIndex < bottomEnd; rowIndex++) {
            // Do update!
            auto curItem = this->item(rowIndex,colIndex);
            if (curItem != nullptr) {
                curItem->setData(PixelDelegateData::HOVER_TYPE,HoverType::NO_HOVER);
            } else {
                // Do nothing if null
            }
        }
    }
}

void DisplayTable::setHover(int row, int column, HoverType hoverType) {
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        return;
    } else {
        curCell->setData(PixelDelegateData::HOVER_TYPE,hoverType);
    }
}

void DisplayTable::doBgBrushClick(QTableWidgetItem *curItem) {
    if (globalSettings.currentBrush == nullptr) {
        YUtils::printDebug("Brush currently null",DebugType::ERROR);
        YUtils::popupAlert("Brush currently null");
        return;
    }
    if (globalSettings.currentBrush->tileAttrs.empty()) {
        YUtils::printDebug("Brush tiles currently empty",DebugType::WARNING);
        YUtils::popupAlert("Brush tiles currently empty");
        return;
    }
    const uint32_t checkTileAttrsSize = BrushTable::CELL_COUNT_DIMS * BrushTable::CELL_COUNT_DIMS;
    if (globalSettings.currentBrush->tileAttrs.size() != checkTileAttrsSize) {
        YUtils::printDebug("Brush tiles size unusual",DebugType::WARNING);
        YUtils::popupAlert("Brush tiles size unusual");
        return;
    }
    auto curScen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground);
    if (curScen == nullptr) {
        YUtils::printDebug("Invalid BG in doBgBrushClick",DebugType::ERROR);
        return;
    }
    std::string curScenImbz = curScen->getInfo()->imbzFilename;
    std::string curBrushImbz = globalSettings.currentBrush->brushTileset;
    if (curScenImbz.compare(curBrushImbz) != 0) {
        std::stringstream ssMismatchImbz;
        ssMismatchImbz << "Mismatch in current tileset vs brush tileset: '";
        ssMismatchImbz << curScenImbz << "' vs '";
        ssMismatchImbz << curBrushImbz << "'";
        YUtils::printDebug(ssMismatchImbz.str(),DebugType::WARNING);
        return;
    }
    uint32_t yBase = curItem->row();
    uint32_t xBase = curItem->column();
    // Round down (TODO add a disable toggle in options?)
    if (yBase % 2 != 0) { // Odd
        yBase--;
    }
    if  (xBase % 2 != 0) { // Odd
        xBase--;
    }
    
    // Do tile placement loop
    QUndoCommand* multiAddCmd = new QUndoCommand();
    for (int y = 0; y < BrushTable::CELL_COUNT_DIMS; y++) {
        for (int x = 0; x < BrushTable::CELL_COUNT_DIMS; x++) {
            uint attrPos = x + y*BrushTable::CELL_COUNT_DIMS;
            auto tileAttr = globalSettings.currentBrush->tileAttrs.at(attrPos);
            if (tileAttr.compile() == 0) {
                continue;
            }
            int rowY = yBase+y;
            int colX = xBase+x;
            new AddTileToGridCommand(rowY,colX,tileAttr,this->yidsRom,this,multiAddCmd);
        }
    }
    emit this->pushStateCommandToStack(multiAddCmd);
    emit this->triggerMainWindowUpdate(); // To mark savable
}

std::vector<QTableWidgetItem*> DisplayTable::getIntersectedTiles(QRect selectionRect) {
    std::vector<QTableWidgetItem*> result;
    if (globalSettings.currentEditingBackground == 0) {
        YUtils::printDebug("getIntersectedTiles does not support non-bg layers",DebugType::ERROR);
        YUtils::popupAlert("getIntersectedTiles does not support non-bg layers");
        return result;
    }
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                auto itemRect = this->visualItemRect(potentialItem);
                if (selectionRect.intersects(itemRect)) {
                    result.push_back(potentialItem);
                }
            }
        }
    }
    if (result.size() < 1) {
        YUtils::printDebug("Selection size was 0",DebugType::WARNING);
        YUtils::popupAlert("Selection size was 0");
    }
    return result;
}

void DisplayTable::updateSelectedTilesVisuals(int whichBg) {
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                potentialItem->setData(PixelDelegateData::TILE_SELECTED_BG1,false);
                potentialItem->setData(PixelDelegateData::TILE_SELECTED_BG2,false);
                potentialItem->setData(PixelDelegateData::TILE_SELECTED_BG3,false);
            }
        }
    }
    for (auto it = globalSettings.selectedItemPointers.begin(); it != globalSettings.selectedItemPointers.end(); it++) {
        if (whichBg == 1) {
            (*it)->setData(PixelDelegateData::TILE_SELECTED_BG1,true);
        } else if (whichBg == 2) {
            (*it)->setData(PixelDelegateData::TILE_SELECTED_BG2,true);
        } else if (whichBg == 3) {
            (*it)->setData(PixelDelegateData::TILE_SELECTED_BG3,true);
        } else {
            YUtils::printDebug("Unusual whichBg",DebugType::ERROR);
        }
    }
}

void DisplayTable::handleSpritesRightClickPress(QMouseEvent *event) {
    //YUtils::printDebug("Right mouse sprites click");
    auto curItemUnderCursor = this->itemAt(event->pos());
    if (curItemUnderCursor == nullptr) {
        YUtils::printDebug("Item under cursor in handleSpritesRightClickPress is null", DebugType::ERROR);
        return;
    }
    auto rowY = curItemUnderCursor->row();
    if (rowY < 0) {
        YUtils::printDebug("rowY less than 0",DebugType::ERROR);
        return;
    }
    auto colX = curItemUnderCursor->column();
    if (colX < 0) {
        YUtils::printDebug("colX less than 0",DebugType::ERROR);
        return;
    }
    // Knock out a few early errors
    this->selectedObjects.clear();
    this->clearSelection();

    uint32_t spriteId = globalSettings.currentSpriteIdToAdd;
    auto spriteMeta = this->yidsRom->getSpriteMetadata(spriteId);
    LevelObject newSprite;
    newSprite.uuid = 0xffff; // UUID is actually set when adding to SETD
    newSprite.xPosition = curItemUnderCursor->column();
    newSprite.yPosition = curItemUnderCursor->row();
    // This is literally the one place you can use createdSettingsLen
    newSprite.settingsLength = spriteMeta.createdSettingsLen;
    newSprite.settings = std::vector<uint8_t>();
    newSprite.settings.resize(newSprite.settingsLength); // Should fill with zeroes
    newSprite.objectId = spriteId;
    AddSpriteCommand *addCmd = new AddSpriteCommand(newSprite,this,this->yidsRom);
    emit this->pushStateCommandToStack(addCmd);
    emit this->triggerMainWindowUpdate();
}

void DisplayTable::updatePortals(bool drawEntrances, bool drawExits) {
    //YUtils::printDebug("Updating level entrances and exits",DebugType::VERBOSE);
    if (this->yidsRom == nullptr) {
        YUtils::printDebug("Cannot update portals, rom is null",DebugType::ERROR);
        return;
    }
    if (this->yidsRom->currentLevelSelectData == nullptr) {
        YUtils::printDebug("Cannot update portals, CRSB data is null",DebugType::ERROR);
        return;
    }
    if (this->yidsRom->mapData == nullptr) {
        YUtils::printDebug("Cannot update portals, map data is null",DebugType::ERROR);
        return;
    }
    if (this->yidsRom->mapData->filename.empty()) {
        YUtils::printDebug("Cannot update portals, no map data file name",DebugType::ERROR);
        return;
    }
    // Update main to match
    this->shouldDrawEntrances = drawEntrances;
    this->shouldDrawExits = drawExits;
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                potentialItem->setData(PixelDelegateData::ENTRANCE_INDEX,0xff);
                potentialItem->setData(PixelDelegateData::ENTRANCE_TYPE,LevelSelectEnums::MapEntranceAnimation::NO_ENTRANCE);
                potentialItem->setData(PixelDelegateData::EXIT_INDEX,0xff);
                potentialItem->setData(PixelDelegateData::EXIT_TYPE,LevelSelectEnums::MapExitStartType::NO_EXIT);
                potentialItem->setData(PixelDelegateData::DRAW_ENTRANCES,drawEntrances);
                potentialItem->setData(PixelDelegateData::DRAW_EXITS,drawExits);
            }
        }
    }
    auto levelData = this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    uint entranceIndex = 0;
    for (auto eit = levelData->entrances.begin(); eit != levelData->entrances.end(); eit++) {
        uint32_t colX = (*eit)->entranceX;
        uint32_t rowY = (*eit)->entranceY;
        auto curItem = this->item(rowY,colX);
        if (curItem == nullptr) {
            YUtils::printDebug("Could not place entrance, tile null",DebugType::ERROR);
            continue;
        }
        int entranceType = (*eit)->enterMapAnimation;
        curItem->setData(PixelDelegateData::ENTRANCE_INDEX,entranceIndex);
        curItem->setData(PixelDelegateData::ENTRANCE_TYPE,entranceType);
        std::stringstream ssEnterTooltip;
        ssEnterTooltip << "Entrance Index: 0x" << std::hex << entranceIndex << std::endl; // New line
        ssEnterTooltip << "Entrance Anim: " << MapEntrance::printEntranceAnimation((*eit)->enterMapAnimation);
        curItem->setToolTip(tr(ssEnterTooltip.str().c_str()));
        entranceIndex++;
    }
    uint exitIndex = 0;
    for (auto xit = levelData->exits.begin(); xit != levelData->exits.end(); xit++) {
        uint32_t colX = (*xit)->exitLocationX;
        uint32_t rowY = (*xit)->exitLocationY;
        auto curItem = this->item(rowY,colX);
        if (curItem == nullptr) {
            YUtils::printDebug("Could not place exit, tile null",DebugType::ERROR);
            continue;
        }
        int exitType = (*xit)->exitStartType;
        curItem->setData(PixelDelegateData::EXIT_INDEX,exitIndex);
        curItem->setData(PixelDelegateData::EXIT_TYPE,exitType);
        std::stringstream ssExitTooltip;
        ssExitTooltip << "Exit Index: 0x" << std::hex << exitIndex << std::endl; // New line
        ssExitTooltip << "Exit Type: " << MapExitData::printExitStartType((*xit)->exitStartType);
        curItem->setToolTip(tr(ssExitTooltip.str().c_str()));
        exitIndex++;
    }
}

void DisplayTable::mousePressEvent(QMouseEvent *event) {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        if (event->button() == Qt::LeftButton) {
            // Something is already selected?
            if (this->selectedObjects.size() > 0) {
                if (this->selectedObjects.size() > 1) {
                    YUtils::printDebug("Multiple object selection not yet supported",DebugType::WARNING);
                }
                uint32_t selectedObjectUuid = this->selectedObjects.at(0);
                auto curItemUnderCursor = this->itemAt(event->pos());
                if (curItemUnderCursor == nullptr) {
                    YUtils::printDebug("Item under cursor in mousePressEvent is null", DebugType::ERROR);
                    return;
                }
                // Do they match?
                auto cursorUuidMaybe = curItemUnderCursor->data(PixelDelegateData::OBJECT_UUID);
                if (cursorUuidMaybe.isNull()) {
                    YUtils::printDebug("Mismatch in selected item and cursor item, deselecting",DebugType::VERBOSE);
                    this->selectedObjects.clear();
                    this->clearVisualSpriteSelection();
                    return;
                }
                if (cursorUuidMaybe.toUInt() == selectedObjectUuid) {
                    YUtils::printDebug("Selected and clicked match! Starting drag...",DebugType::VERBOSE);
                    QPoint underCursorPos(curItemUnderCursor->column(),curItemUnderCursor->row());
                    this->dragStartPosition = underCursorPos;
                    QDrag *drag = new QDrag(this);
                    QMimeData *mimeData = new QMimeData();
                    QByteArray uuidBytes;
                    uuidBytes.setNum(this->selectedObjects.at(0));
                    mimeData->setData("application/x-stork-sprite-uuid",uuidBytes);
                    drag->setMimeData(mimeData);
                    drag->exec(Qt::MoveAction);
                    return;
                } else {
                    //YUtils::printDebug("Clicked a different object, selecting it");
                    this->clearVisualSpriteSelection();
                    this->selectedObjects.clear();
                    this->selectItemByUuid(cursorUuidMaybe.toUInt());
                    return;
                }
            } else {
                // Nothing is selected
                auto curItemUnderCursor = this->itemAt(event->pos());
                if (curItemUnderCursor == nullptr) {
                    YUtils::printDebug("No item loaded under cursor",DebugType::ERROR);
                    return;
                }
                if (!curItemUnderCursor->data(PixelDelegateData::OBJECT_UUID).isNull()) {
                    uint32_t cursorItemUuid = curItemUnderCursor->data(PixelDelegateData::OBJECT_UUID).toUInt();
                    if (cursorItemUuid < 1) {
                        YUtils::printDebug("Cursor Item UUID is less than 1",DebugType::ERROR);
                        return;
                    }
                    // Change the following 2 lines when multiple item selection is enabled
                    this->clearVisualSpriteSelection();
                    this->selectedObjects.clear();

                    //YUtils::printDebug("Doing item selection in mousePressEvent");
                    this->selectItemByUuid(cursorItemUuid);
                    return;
                } else {
                    YUtils::printDebug("Area clicked does not have an item UUID, deselecting all");
                    this->selectedObjects.clear();
                    this->clearVisualSpriteSelection();
                    return;
                }
            }
        } else if (event->button() == Qt::RightButton) {
            this->handleSpritesRightClickPress(event);
        } else {
           YUtils::printDebug("Unhandled mouse click in DisplayTable"); 
        }
    } else if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER
    ) {
        /*******************
         *** BACKGROUNDS ***
         *******************/
        auto curItemUnderCursor = this->itemAt(event->pos());
        if (curItemUnderCursor == nullptr) {
            YUtils::printDebug("Item under cursor in mousePressEvent for BGX is null", DebugType::ERROR);
            return;
        }
        if (globalSettings.layerSelectMode == LayerMode::BG1_LAYER) {
            this->printCellDebug(curItemUnderCursor,1);
        } else if (globalSettings.layerSelectMode == LayerMode::BG2_LAYER) {
            this->printCellDebug(curItemUnderCursor,2);
        } else if (globalSettings.layerSelectMode == LayerMode::BG3_LAYER) {
            this->printCellDebug(curItemUnderCursor,3);
        } else {
            YUtils::printDebug("If this is hit, you seriously messed something up",DebugType::ERROR);
        }
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
            // Right button will erase
            this->clearSelection();
            this->selectorBandOrigin = event->pos();
            this->selectorBand->setGeometry(QRect(this->selectorBandOrigin, QSize()));
            this->selectorBand->show();
            return;
        } else {
            YUtils::printDebug("Unhandled mouse click",DebugType::WARNING);
            return;
        }
    } else if (globalSettings.layerSelectMode == LayerMode::COLLISION_LAYER) {
        auto curItemUnderCursor = this->itemAt(event->pos());
        if (curItemUnderCursor == nullptr) {
            YUtils::printDebug("Item under cursor in mousePressEvent for COLL is null", DebugType::ERROR);
            return;
        }
        if (curItemUnderCursor->data(PixelDelegateData::COLLISION_DRAW).isNull()) {
            YUtils::printDebug("No collision data present");
            return;
        }
        auto colData = curItemUnderCursor->data(PixelDelegateData::COLLISION_DEBUG).toUInt();
        if (event->button() == Qt::LeftButton) {
            uint32_t colX = curItemUnderCursor->column() / 2;
            uint32_t rowY = curItemUnderCursor->row() / 2;
            auto colCmd = new SetCollisionTileCommand(rowY,colX,globalSettings.colTypeToDraw,this->yidsRom,this);
            emit this->pushStateCommandToStack(colCmd);
            emit this->triggerMainWindowUpdate(); // Mark savable update mainly
            return;
        } else if (event->button() == Qt::RightButton) {
            std::stringstream ssColInfo;
            ssColInfo << "Collision data: 0x" << std::hex << colData;
            YUtils::printDebug(ssColInfo.str(),DebugType::VERBOSE);
            globalSettings.colTypeToDraw = (CollisionType)((int)colData);
            std::stringstream ssColStatus;
            auto colName = YUtils::getCollisionMetadata(globalSettings.colTypeToDraw).prettyName;
            ssColStatus << "Setting collision brush to '" << colName << "'";
            emit this->updateMainWindowStatus(ssColStatus.str());
            return;
        } else {
            YUtils::printDebug("Unknown mouse action in collision mode",DebugType::VERBOSE);
        }
    }
    QTableWidget::mousePressEvent(event);
}

void DisplayTable::mouseReleaseEvent(QMouseEvent *event) {
    if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER
    ) {
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
            bool bandBigEnoughForMultiSelect = false;
            auto bandWidth = this->selectorBand->width();
            auto bandHeight = this->selectorBand->height();
            if (bandWidth > DisplayTable::CELL_SIZE_PX || bandHeight > DisplayTable::CELL_SIZE_PX) {
                bandBigEnoughForMultiSelect = true;
            }
            this->selectorBand->hide();

            if (bandBigEnoughForMultiSelect) {
                // For some reason QRubberBand's rect() is broken
                auto bandX = this->selectorBand->x();
                auto bandY = this->selectorBand->y();
                QRect rect(bandX,bandY,this->selectorBand->width(),this->selectorBand->height());
                if (event->button() == Qt::LeftButton) {
                    globalSettings.selectedItemPointers = this->getIntersectedTiles(rect);
                    this->updateSelectedTilesVisuals(globalSettings.currentEditingBackground);
                } else {
                    QUndoCommand* totalRmCmd = new QUndoCommand();
                    auto tilesToWipe = this->getIntersectedTiles(rect);
                    auto blankTileAttr = YUtils::getMapTileRecordDataFromShort(0x0000);
                    for (auto it = tilesToWipe.begin(); it != tilesToWipe.end(); it++) {
                        auto tw = *it;
                        new AddTileToGridCommand(tw->row(),tw->column(),blankTileAttr,this->yidsRom,this,totalRmCmd);
                    }
                    emit this->pushStateCommandToStack(totalRmCmd);
                }
            } else {
                // Do single
                auto curItemUnderCursor = this->itemAt(event->pos());
                if (curItemUnderCursor == nullptr) {
                    YUtils::printDebug("Item under cursor in mouseReleaseEvent for BGX is null", DebugType::ERROR);
                    return;
                }
                if (event->button() == Qt::LeftButton) {
                    this->doBgBrushClick(curItemUnderCursor);
                } else {
                    YUtils::printDebug("TODO: right button single click");
                }
            }
        } else {
            std::cout << "Unknown mouse button release" << std::endl;
        }

    } else {
        QTableWidget::mouseReleaseEvent(event);
    }
}

void DisplayTable::mouseMoveEvent(QMouseEvent *event) {
    if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER
    ) {
        this->selectorBand->setGeometry(QRect(this->selectorBandOrigin, event->pos()).normalized());
        
    }
    // Does too much stuff, such as drag and cell entering, to not do
    QTableWidget::mouseMoveEvent(event);
}

void DisplayTable::dragEnterEvent(QDragEnterEvent *event) {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        if (event->mimeData()->hasFormat("application/x-stork-sprite-uuid")) {
            event->setAccepted(true);
        } else {
            YUtils::printDebug("dragEnterEvent did not detect 'application/x-stork-sprite-uuid'");
            event->setAccepted(false);
        }
    } else {
        YUtils::printDebug("dragEnterEvent not supported for this layer", DebugType::WARNING);
        QTableWidget::dragEnterEvent(event);
    }
    
}

void DisplayTable::dragMoveEvent(QDragMoveEvent *event) {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        if (event->mimeData()->hasFormat("application/x-stork-sprite-uuid")) {
            event->setAccepted(true);
        } else {
            YUtils::printDebug("dragMoveEvent did not detect 'application/x-stork-sprite-uuid'");
            event->setAccepted(false);
        }
    } else {
        YUtils::printDebug("dragMoveEvent not supported for this layer",DebugType::WARNING);
        QTableWidget::dragMoveEvent(event);
    }
}

void DisplayTable::dropEvent(QDropEvent *event) {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        if (event->mimeData()->hasFormat("application/x-stork-sprite-uuid")) {
            auto uuidByteData = event->mimeData()->data("application/x-stork-sprite-uuid");
            uint32_t uuid = uuidByteData.toUInt();
            auto tableItem = this->itemAt(event->pos());
            int tableX = tableItem->column();
            int tableY = tableItem->row();
            auto lo = *this->yidsRom->mapData->getLevelObjectByUuid(uuid);
            MoveSpriteCommand *mov = new MoveSpriteCommand(lo,tableX,tableY,this,this->yidsRom);
            emit this->pushStateCommandToStack(mov);
        } else {
            YUtils::printDebug("dropEvent did not detect 'application/x-stork-sprite-uuid'");
            event->ignore();
        }
    } else {
        YUtils::printDebug("dropEvent not supported for this layer",DebugType::WARNING);
        QTableWidget::dropEvent(event);
    }
}

void DisplayTable::selectItemByUuid(uint32_t uuid, bool triggerMainWindowUpdate) {
    if (globalSettings.layerSelectMode != LayerMode::SPRITES_LAYER) {
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
                potentialItem->setData(PixelDelegateData::SPRITE_SELECTED,true);
            }
        }
    }
    if (triggerMainWindowUpdate) {
        // Links to displayTableUpdate
        emit this->triggerMainWindowUpdate();
    }
}

void DisplayTable::setCellCollision(int row, int column, CollisionDraw colType, uint8_t _colDebug) {
    QTableWidgetItem* curCell = this->item(row,column);
    if (curCell == nullptr) {
        // Make a new one
        curCell = new QTableWidgetItem();
    }
    curCell->setData(PixelDelegateData::COLLISION_DRAW,colType);
    curCell->setData(PixelDelegateData::COLLISION_DEBUG,_colDebug);
}

void DisplayTable::initCellCollision() {
    auto canvasWidthCol = this->yidsRom->mapData->getCollisionCanvasWidth();
    if (canvasWidthCol < 1) {
        YUtils::printDebug("Canvas Width for Collision is invalid!",DebugType::ERROR);
        return;
    }
    const uint32_t cutOff = canvasWidthCol/2;
    auto collisionTileArray = this->yidsRom->mapData->getCollisionData();
    const int CELL_LIST_SIZE = collisionTileArray->colData.size();
    if (CELL_LIST_SIZE < 1) {
        YUtils::printDebug("Collision Tile Array is empty!",DebugType::ERROR);
        return;
    }
    auto colArray = collisionTileArray->colData;
    for (int colIndex = 0; colIndex < CELL_LIST_SIZE; colIndex++) {
        const auto curCol = colArray.at(colIndex);
        uint32_t y = (colIndex / cutOff)*2;
        uint32_t x = (colIndex % cutOff)*2;
        switch ((CollisionType)curCol) {
            case CollisionType::SQUARE: {
                this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT, curCol);
                break;
            }
            case CollisionType::PLATFORM_PASSABLE: {
                this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::ZIG_ZAG, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::ZIG_ZAG, curCol);
                break;
            }
            case CollisionType::DOWN_RIGHT_45: {
                this->setCellCollision(y,  x,  CollisionDraw::DIAG_DOWN_RIGHT, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::DIAG_DOWN_RIGHT, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::CLEAR,curCol);
                break;
            }
            case CollisionType::UP_RIGHT_45: {
                this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::DIAG_UP_RIGHT, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::DIAG_UP_RIGHT, curCol);
                this->setCellCollision(y  ,x  ,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::STATIC_COIN: {
                this->setCellCollision(y  ,x  ,CollisionDraw::COIN_TOP_LEFT, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::COIN_BOTTOM_LEFT, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::COIN_TOP_RIGHT, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::COIN_BOTTOM_RIGHT, curCol);
                break;
            }
            case CollisionType::UP_RIGHT_30: {
                this->setCellCollision(y+1,x  ,CollisionDraw::UP_RIGHT_30_BL, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::UP_RIGHT_30_BR, curCol);
                this->setCellCollision(y  ,x  ,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::UP_RIGHT_30_HALFSTART: {
                this->setCellCollision(y  ,x  ,CollisionDraw::UP_RIGHT_30_BL, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::UP_RIGHT_30_BR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SQUARE_DRAW, curCol);
                break;
            }
            case CollisionType::DOWN_RIGHT_30_1: {
                this->setCellCollision(y  ,x  ,CollisionDraw::DOWN_RIGHT_30_TALL, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::DOWN_RIGHT_30_SHORT, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SQUARE_DRAW, curCol);
                break;
            }
            case CollisionType::DOWN_RIGHT_30_2: {
                this->setCellCollision(y+1,x  ,CollisionDraw::DOWN_RIGHT_30_TALL, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::DOWN_RIGHT_30_SHORT, curCol);
                this->setCellCollision(y  ,x  ,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::UPSIDE_DOWN_UP_RIGHT: {
                this->setCellCollision(y  ,x  ,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::UPSIDE_DOWN_RIGHT_45, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::UPSIDE_DOWN_RIGHT_45, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::UPSIDE_DOWN_UP_30: {
                this->setCellCollision(y+1,x+1,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_SHORT, curCol);
                this->setCellCollision(y  ,x  ,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_TALL, curCol);
                break;
            }
            case CollisionType::UPSIDE_DOWN_UP_30_2: {
                this->setCellCollision(y  ,x  ,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_TALL, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_SHORT, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::UP_RIGHT_STEEP_1: {
                // Side squares
                this->setCellCollision(y  ,x+1,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SQUARE_DRAW, curCol);
                // Repeat steep
                this->setCellCollision(y  ,x  ,CollisionDraw::UP_RIGHT_STEEP_SHORT, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::UP_RIGHT_STEEP_TALL, curCol);
                break;
            }
            case CollisionType::UP_RIGHT_STEEP_2: {
                this->setCellCollision(y  ,x+1,CollisionDraw::UP_RIGHT_STEEP_SHORT, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::UP_RIGHT_STEEP_TALL, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y  ,x  ,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::UPSIDE_DOWN_DOWNWARDS_45: {
                this->setCellCollision(y  ,x+1,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y  ,x  ,CollisionDraw::UPSIDE_DOWN_DOWNWARDS_45_DRAW, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::UPSIDE_DOWN_DOWNWARDS_45_DRAW, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::DOWN_RIGHT_STEEP_HALFSTART: {
                this->setCellCollision(y  ,x  ,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::DOWN_RIGHT_STEEP_THIN, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::DOWN_RIGHT_STEEP_WIDE, curCol);
                break;
            }
            case CollisionType::DOWN_RIGHT_STEEP: {
                this->setCellCollision(y  ,x  ,CollisionDraw::DOWN_RIGHT_STEEP_THIN, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::DOWN_RIGHT_STEEP_WIDE, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::KILL_SPIKES: {
                this->setCellCollision(y,  x,  CollisionDraw::ZIG_ZAG_UPSIDE_DOWN_RED, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::SQERR, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::ZIG_ZAG_UPSIDE_DOWN_RED, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SQERR, curCol);
                break;
            }
            case CollisionType::ICY_SQUARE: {
                this->setCellCollision(y,  x,  CollisionDraw::SLIPPERY_ICE_SQUARE_LIGHTER, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::SLIPPERY_ICE_SQUARE, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::SLIPPERY_ICE_SQUARE, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SLIPPERY_ICE_SQUARE_LIGHTER, curCol);
                break;
            }
            case CollisionType::SOFT_ROCK: {
                this->setCellCollision(y,  x,  CollisionDraw::SOFT_ROCK_SQUARE, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::SOFT_ROCK_SQUARE, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::SOFT_ROCK_SQUARE, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SOFT_ROCK_SQUARE, curCol);
                break;
            }
            case CollisionType::UPSIDE_DOWN_SLOPE_30_1: {
                this->setCellCollision(y,  x,  CollisionDraw::UPSIDE_DOWN_SLOPE_30_1_DRAW, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::UPSIDE_DOWN_SLOPE_30_2_DRAW, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::UPSIDE_DOWN_SLOPE_30_2: {
                this->setCellCollision(y,  x,  CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::SQUARE_DRAW, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::UPSIDE_DOWN_SLOPE_30_1_DRAW, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::UPSIDE_DOWN_SLOPE_30_2_DRAW, curCol);
                break;
            }
            case CollisionType::CLIMBABLE_VINE_CEILING: {
                this->setCellCollision(y,  x,  CollisionDraw::VINE_LEFT, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::VINE_RIGHT, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::CLIMBABLE_VINE_TALL: {
                this->setCellCollision(y,  x,  CollisionDraw::VINE_LEFT, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::VINE_RIGHT, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::VINE_LEFT, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::VINE_RIGHT, curCol);
                break;
            }
            case CollisionType::STAIRS_DOWN_RIGHT: {
                this->setCellCollision(y  ,x  ,CollisionDraw::STAIRS_DOWN_RIGHT_DRAW, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::STAIRS_DOWN_RIGHT_DRAW, curCol);
                this->setCellCollision(y+1,x  ,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y  ,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            case CollisionType::WATER_STILL: {
                this->setCellCollision(y,  x,  CollisionDraw::WATER_STILL_DRAW, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::WATER_STILL_DRAW, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::WATER_STILL_DRAW, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::WATER_STILL_DRAW, curCol);
                break;
            }
            case CollisionType::NONE: {
                this->setCellCollision(y,  x,  CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::CLEAR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
                break;
            }
            default: {
                this->setCellCollision(y,  x,  CollisionDraw::SQERR, curCol);
                this->setCellCollision(y+1,x,  CollisionDraw::SQERR, curCol);
                this->setCellCollision(y,  x+1,CollisionDraw::SQERR, curCol);
                this->setCellCollision(y+1,x+1,CollisionDraw::SQERR, curCol);
                break;
            }
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

void DisplayTable::updateTriggerBoxes() {
    auto triggerBoxDataMaybe = this->yidsRom->mapData->getFirstDataByMagic(Constants::AREA_MAGIC_NUM,true);
    if (triggerBoxDataMaybe == nullptr) {
        //YUtils::printDebug("No TriggerBoxes (AREA) for this map",DebugType::VERBOSE);
        return;
    }
    YUtils::printDebug("Updating TriggerBoxes",DebugType::VERBOSE);
    auto triggerBoxData = static_cast<TriggerBoxData*>(triggerBoxDataMaybe);
    auto triggerBoxes = triggerBoxData->triggers;
    for (auto tit = triggerBoxes.begin(); tit != triggerBoxes.end(); tit++) {
        auto tb = (*tit);
        auto leftX = tb->leftX;
        auto topY = tb->topY;
        auto rightX = tb->rightX;
        auto bottomY = tb->bottomY;

        for (uint x = leftX; x < rightX; x++) {
            for (uint y = topY; y < bottomY; y++) {
                auto curItem = this->item(y,x);
                if (curItem == nullptr) {
                    YUtils::printDebug("Current item in trigger box loop is null",DebugType::WARNING);
                    return;
                }
                if (this->shouldShowTriggers) {
                    auto itemArray = curItem->data(PixelDelegateData::DRAW_TRIGGERS).toByteArray();
                    if (itemArray.isNull()) {
                        itemArray = QByteArray();
                    }
                    itemArray.push_back((uint8_t)tb->uuid);
                    curItem->setData(PixelDelegateData::DRAW_TRIGGERS,itemArray);
                } else {
                    curItem->setData(PixelDelegateData::DRAW_TRIGGERS,QByteArray());
                }
            }
        }
    }
}

void DisplayTable::moveSpriteTo(uint32_t uuid, uint32_t newX, uint32_t newY) {
    //this->wipeObject(uuid);
    this->yidsRom->moveObjectTo(uuid,newX,newY);
    this->updateSprites();
    this->clearVisualSpriteSelection();
    this->selectedObjects.clear();
    this->selectItemByUuid(uuid);
}

bool DisplayTable::placeNewTileOnMap(int row, int column, MapTileRecordData mapRecord, uint32_t whichBg, bool skipPalOffset) {
    //YUtils::printDebug("placeNewTileOnMap()");
    if (row < 0 || row > this->rowCount()-1) {
        YUtils::printDebug("placeNewTileOnMap row out of bounds",DebugType::VERBOSE);
        return false;
    }
    if (column < 0 || column > this->columnCount()-1) {
        YUtils::printDebug("placeNewTileOnMap column out of bounds",DebugType::VERBOSE);
        return false;
    }
    if (whichBg < 1) {
        YUtils::printDebug("whichBg value in placeNewTileOnMap too low",DebugType::ERROR);
        YUtils::popupAlert("whichBg value in placeNewTileOnMap too low");
        return false;
    }
    if (whichBg > 3) {
        YUtils::printDebug("whichBg value in placeNewTileOnMap too high",DebugType::ERROR);
        YUtils::popupAlert("whichBg value in placeNewTileOnMap too high");
        return false;
    }
    if (mapRecord.tileId == 0xffff) {
        // This brush tile is empty, skip silently
        return false;
    }
    auto curItem = this->item(row,column);
    if (curItem == nullptr) {
        YUtils::printDebug("Tile was not present before, creating new");
        curItem = new QTableWidgetItem();
    }
    auto scen = this->yidsRom->mapData->getScenByBg(whichBg,false);
    if (scen == nullptr) {
        YUtils::printDebug("SCEN for this BG is nullptr, skipping",DebugType::WARNING);
        return false;
    }
    // TODO
    if (scen->getInfo()->colorMode == BgColorMode::MODE_256) {
        YUtils::printDebug("256-bit color mode layers not yet supported",DebugType::WARNING);
        YUtils::popupAlert("256-bit color mode layers not yet supported");
        return false;
    }
    // TODO
    if (scen->getInfo()->colorMode == BgColorMode::MODE_UNKNOWN) {
        YUtils::printDebug("0x2 color mode layers not yet supported",DebugType::WARNING);
        YUtils::popupAlert("0x2 color mode layers not yet supported");
        return false;
    }
    auto vramChartiles = this->yidsRom->chartileVram[scen->getInfo()->charBaseBlock];
    Chartile loadedTile;
    try {
        loadedTile = vramChartiles.at(mapRecord.tileId);
    } catch (...) {
        // 0 often just means "empty," but is not consistent
        // Use this as a fallback until you find out
        if (mapRecord.tileId != 0) {
            std::stringstream ssTile;
            ssTile << "Could not get certain tileId for BG" << whichBg << ": " << std::hex << mapRecord.tileId;
            YUtils::printDebug(ssTile.str(),DebugType::ERROR);
        } else {
            YUtils::printDebug("tileId in placeNewTileOnMap was 0",DebugType::WARNING);
        }
        return false;
    }
    auto pal = mapRecord.paletteId;
    if (pal > 16) {
        std::cout << "Palette high pre-offset: 0x" << std::hex << (uint16_t)pal << std::endl;
    }
    if (!skipPalOffset) {
        pal += (uint8_t)this->yidsRom->chartileVramPaletteOffset[scen->getInfo()->charBaseBlock];
    }
    if (pal > 16) {
        std::cout << "Palette high post-offset: 0x" << std::hex << (uint16_t)pal << std::endl;
    }
    auto colorMode = scen->getInfo()->colorMode;
    if (whichBg == 1) {
        // BG 1 //
        curItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
        if (colorMode == BgColorMode::MODE_16 || colorMode == BgColorMode::MODE_UNKNOWN) {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,scen->getPalette()->extendedPalette);
        } // TODO: Check for other color modes and error
        curItem->setData(PixelDelegateData::PALETTE_ID_BG1,pal);
        curItem->setData(PixelDelegateData::FLIP_H_BG1,mapRecord.flipH);
        curItem->setData(PixelDelegateData::FLIP_V_BG1,mapRecord.flipV);
        curItem->setData(PixelDelegateData::TILEATTR_BG1,(uint)mapRecord.tileAttr);
        curItem->setData(PixelDelegateData::TILE_ID_BG1,(uint)mapRecord.tileId);
    } else if (whichBg == 2) {
        // BG 2 //
        curItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
        if (colorMode == BgColorMode::MODE_16 || colorMode == BgColorMode::MODE_UNKNOWN) {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,scen->getPalette()->extendedPalette);
        }
        curItem->setData(PixelDelegateData::PALETTE_ID_BG2,pal);
        curItem->setData(PixelDelegateData::FLIP_H_BG2,mapRecord.flipH);
        curItem->setData(PixelDelegateData::FLIP_V_BG2,mapRecord.flipV);
        curItem->setData(PixelDelegateData::TILEATTR_BG2,(uint)mapRecord.tileAttr);
        curItem->setData(PixelDelegateData::TILE_ID_BG2,(uint)mapRecord.tileId);
    } else if (whichBg == 3) {
        // BG 3 //
        curItem->setData(PixelDelegateData::PIXEL_ARRAY_BG3,loadedTile.tiles);
        if (colorMode == BgColorMode::MODE_16 || colorMode == BgColorMode::MODE_UNKNOWN) {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,scen->getPalette()->extendedPalette);
        }
        curItem->setData(PixelDelegateData::PALETTE_ID_BG3,pal);
        curItem->setData(PixelDelegateData::FLIP_H_BG3,mapRecord.flipH);
        curItem->setData(PixelDelegateData::FLIP_V_BG3,mapRecord.flipV);
        curItem->setData(PixelDelegateData::TILEATTR_BG3,(uint)mapRecord.tileAttr);
        curItem->setData(PixelDelegateData::TILE_ID_BG3,(uint)mapRecord.tileId);
    } else {
        YUtils::printDebug("Unusual whichBg in placeNewTileOnMap",DebugType::ERROR);
        YUtils::popupAlert("Unusual whichBg in placeNewTileOnMap");
        return false;
    }
    curItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
    curItem->setData(PixelDelegateData::HOVER_TYPE,HoverType::NO_HOVER);
    // You are changing the MAP TILES not the pixels you dingus. VRAM is moot
    auto mpbzMaybe = scen->getFirstDataByMagic(Constants::MPBZ_MAGIC_NUM);
    auto mpbz = static_cast<MapTilesData*>(mpbzMaybe);
    uint32_t index = column + (row * scen->getInfo()->layerWidth);
    mpbz->mapTiles.at(index) = mapRecord;
    return true;
}

void DisplayTable::clearVisualSpriteSelection() {
    const int ROW_COUNT = this->rowCount();
    const int COLUMN_COUNT = this->columnCount();
    for (int row = 0; row < ROW_COUNT; row++) {
        for (int col = 0; col < COLUMN_COUNT; col++) {
            auto potentialItem = this->item(row,col);
            if (potentialItem != nullptr) {
                potentialItem->setData(PixelDelegateData::SPRITE_SELECTED,false);
            }
        }
    }
    // Just in case there was the default selection somehow
    this->clearSelection();
}

void DisplayTable::setLayerDraw(uint whichLayer, bool shouldDraw) {
    if (whichLayer == 1) {
        this->drawBg1 = shouldDraw;
    } else if (whichLayer == 2) {
        this->drawBg2 = shouldDraw;
    } else if (whichLayer == 3) {
        this->drawBg3 = shouldDraw;
    } else if (whichLayer == 4) { // Use 4 for objects
        this->drawObjects = shouldDraw;
    } else {
        YUtils::printDebug("Unknown layer to draw used",DebugType::ERROR);
        return;
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
                } else if (whichLayer == 4) {
                    potentialItem->setData(PixelDelegateData::DRAW_OBJECTS,this->drawObjects);
                }
            }
        }
    }
}

void DisplayTable::updateBg() {
    this->yidsRom->reloadChartileVram(0);
    this->yidsRom->mapData->wipeLayerOrderCache();
    auto newCanvasHeight = this->yidsRom->mapData->getGreatestCanvasHeight();
    if (newCanvasHeight == 0) {
        YUtils::printDebug("Canvas height calculation failed!",DebugType::ERROR);
        YUtils::popupAlert("Max canvas height calculation failed");
    } else {
        this->setRowCount(newCanvasHeight);
    }
    
    auto newCanvasWidth = this->yidsRom->mapData->getGreatestCanvasWidth();
    if (newCanvasWidth == 0) {
        YUtils::printDebug("Canvas width calculation failed!",DebugType::ERROR);
        YUtils::popupAlert("Max canvas width calculation failed");
    } else {
        this->setColumnCount(newCanvasWidth);
    }

    for (uint8_t bgIndex = 1; bgIndex <= 3; bgIndex++) {
        std::stringstream ssDoBg;
        ssDoBg << "Updating background " << (uint16_t)bgIndex;
        YUtils::printDebug(ssDoBg.str(),DebugType::VERBOSE);
        // Update the main window
        emit this->updateMainWindowStatus(ssDoBg.str());
        auto curScen = this->yidsRom->mapData->getScenByBg(bgIndex, true);
        if (curScen == nullptr) {
            std::stringstream ssNoScen;
            ssNoScen << "No SCEN file for background " << std::hex;
            ssNoScen << (uint16_t)bgIndex << " found";
            // Handle better?
            YUtils::printDebug(ssNoScen.str(),DebugType::WARNING);
            continue; // Proceed to next BG
        }
        auto mapTiles = curScen->getMapTiles();
        if (mapTiles.size() == 0) {
            std::stringstream ssEmptyPreRender;
            ssEmptyPreRender << "No MPBZ file for background " << std::hex;
            ssEmptyPreRender << (uint16_t)bgIndex << " found";
            YUtils::printDebug(ssEmptyPreRender.str(),DebugType::WARNING);
            continue; // Proceed to next BG
        }
        const uint32_t preRenderSize = mapTiles.size();
        const uint32_t cutOffBg = curScen->getInfo()->layerWidth;
        uint32_t bgLeftOffset = 0;
        // This previously used newCanvasWidth
        uint32_t canvasWidth = this->yidsRom->mapData->getScenByBg(bgIndex)->getInfo()->layerWidth;
        while (bgLeftOffset < canvasWidth) {
            for (uint32_t preRenderIndex = 0; preRenderIndex < preRenderSize; preRenderIndex++) {
                uint32_t y = preRenderIndex / cutOffBg;
                uint32_t x = (preRenderIndex % cutOffBg) + bgLeftOffset;
                // Note: You might need to apply this to other layers later
                // -1 because if you go the full length, it loops back around. Remember, .length - 1!
                if (x > canvasWidth-1) {
                    std::cout << "X too big (x vs width): " << std::hex << x << " vs " << std::hex << (canvasWidth-1) << std::endl;
                    continue;
                }
                this->putTileBg(x,y,mapTiles.at(preRenderIndex),bgIndex);
            }
            bgLeftOffset += cutOffBg;
        }
        // We want to put the layer drawing properties ON the tiles initially,
        // but not to overwrite it repeatedly when updating the BG
        if (!this->firstLayerDrawDone) {
            this->setLayerDraw(bgIndex,true);
        }
    }
    emit this->updateMainWindowStatus("All backgrounds updated");
    // You've put the layer draw data on every tile now, no need to do it again
    this->firstLayerDrawDone = true;
}

void DisplayTable::updateSprites() {
    //YUtils::printDebug("updateSprites");
    // Full wipe
    uint32_t xWidth = this->columnCount();
    uint32_t yHeight = this->rowCount();
    for (uint32_t col = 0; col < xWidth; col++) {
        for (uint32_t row = 0; row < yHeight; row++) {
            auto currentItem = this->item(row,col);
            if (currentItem != nullptr) {
                currentItem->setData(PixelDelegateData::OBJECT_UUID,QVariant::fromValue(nullptr));
                currentItem->setData(PixelDelegateData::OBJECT_ID,QVariant::fromValue(nullptr));
                currentItem->setData(PixelDelegateData::OBJECT_PALETTE,QVariant::fromValue(nullptr));
                currentItem->setData(PixelDelegateData::OBJECT_TILES,QVariant::fromValue(nullptr));
            }
        }
    }
    //auto t1 = std::chrono::high_resolution_clock::now();
    if (this->yidsRom->mapData->getAllLevelObjects().size() == 0) {
        YUtils::printDebug("No objects loaded, cannot update",DebugType::ERROR);
        return;
    }
    auto loadedLevelObjects = this->yidsRom->mapData->getAllLevelObjects();
    for (auto itp = loadedLevelObjects.cbegin(); itp != loadedLevelObjects.cend(); itp++) {
        auto it = (*itp); // Convert to normal reference
        auto x = it->xPosition;
        auto y = it->yPosition;
        auto potentialExisting = this->item(y,x);
        if (potentialExisting == nullptr) {
            potentialExisting = new QTableWidgetItem();
        }
        auto objectGraphicsMeta = LevelObject::getObjectGraphicMetadata(*it);
        // if (it->objectId == 0x94) {
        //     std::cout << objectGraphicsMeta.whichObjectFile << std::endl;
        // }
        auto objectTextMeta = this->yidsRom->getSpriteMetadata(it->objectId);
        if (objectGraphicsMeta.indexOfPalette == 0 && objectGraphicsMeta.indexOfTiles == 0) {
            // top left I think
            potentialExisting->setData(PixelDelegateData::OBJECT_ID,(uint32_t)it->objectId);
            potentialExisting->setData(PixelDelegateData::OBJECT_UUID,it->uuid);
            potentialExisting->setText("sprite");
            potentialExisting->setData(PixelDelegateData::OBJECT_PALETTE,this->yidsRom->backgroundPalettes[0]);
            std::stringstream ss;
            ss << "0x" << std::hex << (uint32_t)it->objectId;
            ss << std::endl;
            ss << objectTextMeta.name;
            ss << std::endl;
            ss << objectTextMeta.info;
            potentialExisting->setToolTip(tr(ss.str().c_str()));
            // Top Right
            auto topRight = this->item(y,x+1);
            if (topRight == nullptr) {
                topRight = new QTableWidgetItem();
            }
            topRight->setData(PixelDelegateData::OBJECT_ID,(uint32_t)it->objectId);
            topRight->setData(PixelDelegateData::OBJECT_UUID,it->uuid);
            topRight->setText("sprite");
            topRight->setData(PixelDelegateData::OBJECT_PALETTE,this->yidsRom->backgroundPalettes[0]);
            topRight->setToolTip(tr(ss.str().c_str()));
            // Bottom Left
            auto bottomLeft = this->item(y+1,x);
            if (bottomLeft == nullptr) {
                bottomLeft = new QTableWidgetItem();
            }
            bottomLeft->setData(PixelDelegateData::OBJECT_ID,(uint32_t)it->objectId);
            bottomLeft->setData(PixelDelegateData::OBJECT_UUID,it->uuid);
            bottomLeft->setText("sprite");
            bottomLeft->setData(PixelDelegateData::OBJECT_PALETTE,this->yidsRom->backgroundPalettes[0]);
            bottomLeft->setToolTip(tr(ss.str().c_str()));
            // Bottom Right
            auto bottomRight = this->item(y+1,x+1);
            if (bottomRight == nullptr) {
                bottomRight = new QTableWidgetItem();
            }
            bottomRight->setData(PixelDelegateData::OBJECT_ID,(uint32_t)it->objectId);
            bottomRight->setData(PixelDelegateData::OBJECT_UUID,it->uuid);
            bottomRight->setText("sprite");
            bottomRight->setData(PixelDelegateData::OBJECT_PALETTE,this->yidsRom->backgroundPalettes[0]);
            bottomRight->setToolTip(tr(ss.str().c_str()));
        } else {
            this->placeObjectGraphic(
                (uint32_t)x,(uint32_t)y,
                objectGraphicsMeta.indexOfTiles,
                objectGraphicsMeta.frame,
                objectGraphicsMeta.indexOfPalette,
                objectGraphicsMeta.whichPaletteFile,
                objectGraphicsMeta.whichObjectFile,
                objectGraphicsMeta.xPixelOffset,
                objectGraphicsMeta.yPixelOffset,
                it->uuid,
                objectGraphicsMeta.isLz10,
                objectGraphicsMeta.forceFlipH,
                objectGraphicsMeta.forceFlipV
            );
            if (objectGraphicsMeta.extras.size() > 0) {
                for (auto eit = objectGraphicsMeta.extras.begin(); eit != objectGraphicsMeta.extras.end(); eit++) {
                    this->placeObjectGraphic(
                        (uint32_t)x,(uint32_t)y, // Has its own offsets from the base
                        eit->indexOfTiles,
                        eit->frame,
                        eit->indexOfPalette,
                        eit->whichPaletteFile,
                        eit->whichObjectFile,
                        eit->xPixelOffset,
                        eit->yPixelOffset,
                        it->uuid, // Absolutely must be kept the same
                        eit->isLz10,
                        eit->forceFlipH,
                        eit->forceFlipV,
                        true
                    );
                }
            }
        }
    }
    // // Timing
    // auto t2 = std::chrono::high_resolution_clock::now();
    // auto lol1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1.time_since_epoch()).count();
    // auto lol2 = std::chrono::duration_cast<std::chrono::milliseconds>(t2.time_since_epoch()).count();
    // std::cout << "milliseconds: " << lol2-lol1 << std::endl;

    emit this->triggerMainWindowUpdate(); // displayTableUpdate
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

struct SpritePlacementStep {
    uint32_t x;
    uint32_t y;
    QByteArray objChartile;
    uint buildFrameIndex;
    void flipV(double midY) {
        int doubled = (int)(midY * 2);
        this->y = doubled - this->y;
    };
    void flipH(double midX) {
        int doubled = (int)(midX * 2);
        this->x = doubled - this->x;
    }
};

void DisplayTable::placeObjectGraphic(
    uint32_t x, uint32_t y,
    uint32_t objectOffset, uint32_t frame,
    uint32_t paletteOffset, std::string paletteFile,
    std::string objectFile,
    int manualXoffsetFine, int manualYoffsetFine,
    uint32_t uuid, bool isLz10,
    bool forceFlipH, bool forceFlipV,
    bool isExtra)
{
    Q_UNUSED(isExtra); // Perhaps use for debug someday
    // std::cout << "placeObjectGraphic" << std::endl;
    // Will skip if already loaded
    this->yidsRom->loadSpriteRenderFile(objectFile);
    this->yidsRom->loadSpriteRenderFile(paletteFile);
    
    QByteArray objectPalette = this->yidsRom->backgroundPalettes[0]; // Default
    try {
        auto paletteDataList = this->yidsRom->spriteRenderFiles[paletteFile]->objectPaletteDataMap;
        // std::cout << "paletteDataList size: 0x" << std::hex << paletteDataList.size() << std::endl;
        // std::cout << "paletteOffset: 0x" << std::hex << paletteOffset << std::endl;
        auto curPalData = paletteDataList.at(paletteOffset);
        objectPalette = curPalData->palettes.at(0);
    } catch (...) {
        std::stringstream ssPalFail;
        ssPalFail << "Failed to get object palette for object with uuid 0x" << std::hex << uuid;
        ssPalFail << ", using backgroundPalettes[0]";
        YUtils::printDebug(ssPalFail.str(),DebugType::ERROR);
    }

    auto tileDataMap = this->yidsRom->spriteRenderFiles[objectFile]->objectTileDataMap;
    if (tileDataMap.count(objectOffset) == 0) {
        std::stringstream ssCharFail;
        ssCharFail << "Failed to get object chartiles for object with uuid 0x" << std::hex << uuid;
        ssCharFail << ", canceling placement";
        YUtils::printDebug(ssCharFail.str(),DebugType::ERROR);
        return;
    }
    auto objectTileData = tileDataMap.at(objectOffset);
    auto curFrame = objectTileData->getFrameAt(frame);

    if (objectPalette.size() < 0xf) {
        std::stringstream ssEmptyPalette;
        ssEmptyPalette << "Palette size is too small! Size: 0x";
        ssEmptyPalette << std::hex << objectPalette.size();
        ssEmptyPalette << ", paletteFile value is 0x" << std::hex << paletteFile;
        ssEmptyPalette << ", object uuid is 0x" << std::hex << uuid;
        YUtils::printDebug(ssEmptyPalette.str(),DebugType::ERROR);
    }
    bool shouldFlipV = false;
    bool shouldFlipH = false;
    uint buildFrameIndex = 0;
    int minY = 0xffffff;
    int maxY = 0x000000;
    int minX = 0xffffff;
    int maxX = 0x000000;
    std::vector<SpritePlacementStep> steps;
    for (auto bit = curFrame.buildFrames.begin(); bit != curFrame.buildFrames.end(); bit++) {
        uint16_t flags = (*bit)->flags;
        auto tileShapeValue = flags & 0b11111;
        QPoint dims = YUtils::getSpriteDimsFromFlagValue(tileShapeValue);
        if (buildFrameIndex == 0) {
            // First flags override all
            shouldFlipV = (flags & 0b0010'0000'0000'0000) != 0;
            shouldFlipH = (flags & 0b0001'0000'0000'0000) != 0;
            // Others don't even have any effect
        }
        int curSpriteWidth = dims.x();
        int curSpriteHeight = dims.y();
        int buildFrameOffsetXfine = (*bit)->xOffset;
        int buildFrameOffsetYfine = (*bit)->yOffset;
        double xd8 = static_cast<double>(buildFrameOffsetXfine+manualXoffsetFine);
        int xd = (int)std::floor(xd8/8);
        double yd8 = static_cast<double>(buildFrameOffsetYfine+manualYoffsetFine);
        int yd = (int)std::floor(yd8/8);
        // if (uuid == 0x01) {
        //     std::cout << "> UUID 0x1 detected" << std::endl;
        //     std::cout << "Flags: 0x" << std::hex << flags << std::endl;
        //     std::cout << "curSpriteWidth: 0x" << std::hex << curSpriteWidth << std::endl;
        //     std::cout << "curSpriteHeight: 0x" << std::hex << curSpriteHeight << std::endl;
        //     std::cout << "xd: " << std::dec << xd << std::endl;
        //     std::cout << "yd: " << std::dec << yd << std::endl;
        // }
        std::vector<QByteArray> tiles = std::vector<QByteArray>();
        if (isLz10) {
            tiles = objectTileData->getChartilesCompressed((*bit)->tileOffset << 4,curSpriteHeight*curSpriteWidth,BgColorMode::MODE_16);
        } else {
            tiles = objectTileData->getChartiles((*bit)->tileOffset << 4,curSpriteHeight*curSpriteWidth,BgColorMode::MODE_16);
        }
        int tilesSize = tiles.size();
        for (int tilesIndex = 0; tilesIndex < tilesSize; tilesIndex++) {
            auto objChartile = tiles.at(tilesIndex);
            int tileIndexOffsetX = tilesIndex % curSpriteWidth;
            int tileIndexOffsetY = tilesIndex / curSpriteWidth;
            // x position on map, offset by buildframe x and inner tile-build x
            int finalX = x+xd+tileIndexOffsetX;
            int finalY = y+yd+tileIndexOffsetY;
            if (finalY > maxY) {
                maxY = finalY;
            }
            if (finalY < minY) {
                minY = finalY;
            }
            if (finalX > maxX) {
                maxX = finalX;
            }
            if (finalX < minX) {
                minX = finalX;
            }
            SpritePlacementStep step;
            step.x = finalX;
            step.y = finalY;
            step.objChartile = objChartile;
            step.buildFrameIndex = buildFrameIndex;
            steps.push_back(step);
        }
        buildFrameIndex++;
    }
    if (forceFlipH) {
        shouldFlipH = true;
    }
    if (forceFlipV) {
        shouldFlipV = true;
    }
    double midY = ((double)(minY + maxY)) / 2;
    double midX = ((double)(minX + maxX)) / 2;
    for (auto fit = steps.begin(); fit != steps.end(); fit++) {
        auto step = *fit;
        if (shouldFlipV) {
            step.flipV(midY);
        }
        if (shouldFlipH) {
            step.flipH(midX);
        }
        auto tileItem = this->item(step.y,step.x);
        if (tileItem == nullptr) {
            tileItem = new QTableWidgetItem();
        }
        // TODO: Make this optional for performance purposes
        bool dontPlaceInvisibles = true;
        if (dontPlaceInvisibles == true) {
            uint total = 0;
            for (auto oit = step.objChartile.begin(); oit != step.objChartile.end(); oit++) {
                total += (uint)(*oit);
            }
            if (total == 0) {
                continue;
            }
        }
        tileItem->setData(PixelDelegateData::OBJECT_TILES,step.objChartile);
        tileItem->setData(PixelDelegateData::OBJECT_PALETTE,objectPalette);
        tileItem->setData(PixelDelegateData::OBJECT_UUID,uuid);
        tileItem->setData(PixelDelegateData::OBJECT_TILES_FLIPV,shouldFlipV);
        tileItem->setData(PixelDelegateData::OBJECT_TILES_FLIPH,shouldFlipH);
        tileItem->setData(PixelDelegateData::OBJECT_TILES_BUILDFRAME_INDEX,step.buildFrameIndex);
        tileItem->setText("sprite");
    }
}
