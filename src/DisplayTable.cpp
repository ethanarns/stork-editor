#include "DisplayTable.h"
#include "PixelDelegate.h"
#include "PixelDelegateEnums.h"
#include "yidsrom.h"
#include "utils.h"
#include "popups/BrushTable.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QDrag>
#include <QApplication>
#include <QRubberBand>

#include <iostream>
#include <sstream>

DisplayTable::DisplayTable(QWidget* parent,YidsRom* rom) {
    Q_UNUSED(parent);
    this->shouldShowCollision = true;
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
    auto vramChartiles = scen->getVramChartiles();
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

    pal += scen->paletteStartOffset - 1; // -1 is likely because of universal palette

    auto bgItem = this->item(y,x);
    auto isColorMode256 = scen->getInfo()->colorMode == BgColorMode::MODE_256;
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
        if (!isColorMode256) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            // Note: the 256 palettes thing does not always start at 0x10 (including the +1)
            // 1-3, there's a palette missing from the palette screen that made this start at 0xf
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(pal+1));
        }
        bgItem->setData(PixelDelegateData::FLIP_H_BG2,pren.flipH);
        bgItem->setData(PixelDelegateData::FLIP_V_BG2,pren.flipV);
        bgItem->setData(PixelDelegateData::TILEATTR_BG2,(uint)pren.tileAttr);
        bgItem->setData(PixelDelegateData::TILE_ID_BG2,(uint)pren.tileId);
    } else if (whichBg == 1) {
        // BG 1 //
        bgItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
        if (!isColorMode256) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->get256Palettes(pal+1));
        }
        bgItem->setData(PixelDelegateData::FLIP_H_BG1,pren.flipH);
        bgItem->setData(PixelDelegateData::FLIP_V_BG1,pren.flipV);
        bgItem->setData(PixelDelegateData::TILEATTR_BG1,(uint)pren.tileAttr);
        bgItem->setData(PixelDelegateData::TILE_ID_BG1,(uint)pren.tileId);
    } else if (whichBg == 3) {
        // BG 3 //
        bgItem->setData(PixelDelegateData::PIXEL_ARRAY_BG3,loadedTile.tiles);
        if (!isColorMode256) {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            bgItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->get256Palettes(pal+1));
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
    for (int y = 0; y < BrushTable::CELL_COUNT_DIMS; y++) {
        for (int x = 0; x < BrushTable::CELL_COUNT_DIMS; x++) {
            uint attrPos = x + y*BrushTable::CELL_COUNT_DIMS;
            auto tileAttr = globalSettings.currentBrush->tileAttrs.at(attrPos);
            if (tileAttr.compile() == 0) {
                continue;
            }
            this->placeNewTileOnMap(yBase+y,xBase+x,tileAttr);
        }
    }
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
                if (selectionRect.contains(itemRect)) {
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

void DisplayTable::mousePressEvent(QMouseEvent *event) {
    if (globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
        // Something is already selected
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

                YUtils::printDebug("Doing item selection in mousePressEvent");
                this->selectItemByUuid(cursorItemUuid);
                return;
            } else {
                YUtils::printDebug("Area clicked does not have an item UUID, deselecting all");
                this->selectedObjects.clear();
                this->clearVisualSpriteSelection();
                return;
            }
        }
    /*******************
     *** BACKGROUNDS ***
     *******************/
    } else if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER
    ) {
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
        if (event->button() == Qt::LeftButton) {
            // TODO start rectangle band
            this->selectorBandOrigin = event->pos();
            this->selectorBand->setGeometry(QRect(this->selectorBandOrigin, QSize()));
            this->selectorBand->show();
            return;
        } else if (event->button() == Qt::RightButton) {
            YUtils::printDebug("TODO: Right click handling");
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
        auto colData = curItemUnderCursor->data(PixelDelegateData::COLLISION_DEBUG).toInt();
        std::cout << "Collision data: 0x" << std::hex << colData << std::endl;
        // Get scen data TODO MAKE THIS LAYER AGNOSTIC
        auto scen = this->yidsRom->mapData->getScenByBg(2,true);
        if (scen == nullptr) {
            YUtils::printDebug("Layer is null",DebugType::WARNING);
            YUtils::popupAlert("Layer is null");
            return;
        }
        auto collisionData = this->yidsRom->mapData->getCollisionData();
        // Coll is in 2x2 segments
        uint32_t layerWidth = scen->getInfo()->layerWidth / 2;
        uint32_t colX = curItemUnderCursor->column() / 2;
        uint32_t rowY = curItemUnderCursor->row() / 2;
        uint32_t posInColArray = colX + (rowY * layerWidth);
        collisionData->colData.at(posInColArray) = globalSettings.colTypeToDraw;
        this->initCellCollision();
        emit this->triggerMainWindowUpdate();
        return;
    }
    QTableWidget::mousePressEvent(event);
}

void DisplayTable::mouseReleaseEvent(QMouseEvent *event) {
    if (
        globalSettings.layerSelectMode == LayerMode::BG1_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG2_LAYER ||
        globalSettings.layerSelectMode == LayerMode::BG3_LAYER
    ) {
        if (event->button() == Qt::LeftButton) {
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
                globalSettings.selectedItemPointers = this->getIntersectedTiles(rect);
                this->updateSelectedTilesVisuals(globalSettings.currentEditingBackground);
            } else {
                // Do single
                auto curItemUnderCursor = this->itemAt(event->pos());
                if (curItemUnderCursor == nullptr) {
                    YUtils::printDebug("Item under cursor in mouseReleaseEvent for BGX is null", DebugType::ERROR);
                    return;
                }
                this->doBgBrushClick(curItemUnderCursor);
            }
        } else if (event->button() == Qt::RightButton) {
            std::cout << "Right mouse button release" << std::endl;
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
    } else {
        QTableWidget::mouseMoveEvent(event);
    }
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
            this->moveSpriteTo(uuid,tableX,tableY);
            this->updateSprites();
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
        if (curCol == CollisionType::SQUARE) {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT, curCol);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT, curCol);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT, curCol);
        } else if (curCol == CollisionType::PLATFORM_PASSABLE) {
            this->setCellCollision(y,  x,  CollisionDraw::CORNER_TOP_LEFT, curCol);
            this->setCellCollision(y,  x+1,CollisionDraw::CORNER_TOP_RIGHT, curCol);
            this->setCellCollision(y+1,x,  CollisionDraw::ZIG_ZAG, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::ZIG_ZAG, curCol);
        } else if (curCol == CollisionType::DOWN_RIGHT_45) {
            this->setCellCollision(y,  x,  CollisionDraw::DIAG_DOWN_RIGHT, curCol);
            this->setCellCollision(y+1,x,  CollisionDraw::CORNER_BOTTOM_LEFT, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::DIAG_DOWN_RIGHT, curCol);
        } else if (curCol == CollisionType::UP_RIGHT_45) {
            this->setCellCollision(y+1,x+1,CollisionDraw::CORNER_BOTTOM_RIGHT, curCol);
            this->setCellCollision(y+1,x,  CollisionDraw::DIAG_UP_RIGHT, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::DIAG_UP_RIGHT, curCol);
        } else if (curCol == CollisionType::STATIC_COIN) {
            this->setCellCollision(y  ,x  ,CollisionDraw::COIN_TOP_LEFT, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::COIN_BOTTOM_LEFT, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::COIN_TOP_RIGHT, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::COIN_BOTTOM_RIGHT, curCol);
        } else if (curCol == CollisionType::UP_RIGHT_30) {
            this->setCellCollision(y+1,x  ,CollisionDraw::UP_RIGHT_30_BL, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::UP_RIGHT_30_BR, curCol);
        } else if (curCol == CollisionType::UP_RIGHT_30_HALFSTART) {
            this->setCellCollision(y  ,x  ,CollisionDraw::UP_RIGHT_30_BL, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::UP_RIGHT_30_BR, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::SQUARE_DRAW, curCol);
        } else if (curCol == CollisionType::DOWN_RIGHT_30_1) {
            this->setCellCollision(y  ,x  ,CollisionDraw::DOWN_RIGHT_30_TALL, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::DOWN_RIGHT_30_SHORT, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::SQUARE_DRAW, curCol);
        } else if (curCol == CollisionType::DOWN_RIGHT_30_2) {
            this->setCellCollision(y+1,x  ,CollisionDraw::DOWN_RIGHT_30_TALL, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::DOWN_RIGHT_30_SHORT, curCol);
        } else if (curCol == CollisionType::UPSIDE_DOWN_UP_RIGHT) {
            this->setCellCollision(y  ,x  ,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::UPSIDE_DOWN_RIGHT_45, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::UPSIDE_DOWN_RIGHT_45, curCol);
        } else if (curCol == CollisionType::UPSIDE_DOWN_UP_30) {
            this->setCellCollision(y+1,x+1,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_SHORT, curCol);
            this->setCellCollision(y  ,x  ,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_TALL, curCol);
        } else if (curCol == CollisionType::UPSIDE_DOWN_UP_30_2) {
            this->setCellCollision(y  ,x  ,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_TALL, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_SHORT, curCol);
        } else if (curCol == CollisionType::UP_RIGHT_STEEP_1) {
            // Side squares
            this->setCellCollision(y  ,x+1,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::SQUARE_DRAW, curCol);
            // Repeat steep
            this->setCellCollision(y  ,x  ,CollisionDraw::UP_RIGHT_STEEP_SHORT, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::UP_RIGHT_STEEP_TALL, curCol);
        } else if (curCol == CollisionType::UP_RIGHT_STEEP_2) {
            this->setCellCollision(y  ,x+1,CollisionDraw::UP_RIGHT_STEEP_SHORT, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::UP_RIGHT_STEEP_TALL, curCol);
        } else if (curCol == CollisionType::UPSIDE_DOWN_DOWNWARDS_45) {
            this->setCellCollision(y  ,x+1,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y  ,x  ,CollisionDraw::UPSIDE_DOWN_DOWNWARDS_45_DRAW, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::UPSIDE_DOWN_DOWNWARDS_45_DRAW, curCol);
        } else if (curCol == CollisionType::DOWN_RIGHT_STEEP_HALFSTART) {
            // do this
            this->setCellCollision(y  ,x  ,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::SQUARE_DRAW, curCol);
            this->setCellCollision(y  ,x+1,CollisionDraw::DOWN_RIGHT_STEEP_THIN, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::DOWN_RIGHT_STEEP_WIDE, curCol);
        } else if (curCol == CollisionType::DOWN_RIGHT_STEEP) {
            this->setCellCollision(y  ,x  ,CollisionDraw::DOWN_RIGHT_STEEP_THIN, curCol);
            this->setCellCollision(y+1,x  ,CollisionDraw::DOWN_RIGHT_STEEP_WIDE, curCol);
        } else if (curCol == CollisionType::NONE) {
            this->setCellCollision(y,  x,  CollisionDraw::CLEAR, curCol);
            this->setCellCollision(y+1,x,  CollisionDraw::CLEAR, curCol);
            this->setCellCollision(y,  x+1,CollisionDraw::CLEAR, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::CLEAR, curCol);
        } else if (curCol != 0) { // Unknown, draw temp
            this->setCellCollision(y,  x,  CollisionDraw::SQERR, curCol);
            this->setCellCollision(y+1,x,  CollisionDraw::SQERR, curCol);
            this->setCellCollision(y,  x+1,CollisionDraw::SQERR, curCol);
            this->setCellCollision(y+1,x+1,CollisionDraw::SQERR, curCol);
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
    this->wipeObject(uuid);
    this->yidsRom->moveObjectTo(uuid,newX,newY);
    this->updateSprites();
    this->clearVisualSpriteSelection();
    this->selectedObjects.clear();
    this->selectItemByUuid(uuid);
}

bool DisplayTable::placeNewTileOnMap(int row, int column, MapTileRecordData mapRecord) {
    //YUtils::printDebug("placeNewTileOnMap()");
    if (row < 0 || row > this->rowCount()-1) {
        YUtils::printDebug("placeNewTileOnMap row out of bounds",DebugType::VERBOSE);
        return false;
    }
    if (column < 0 || column > this->columnCount()-1) {
        YUtils::printDebug("placeNewTileOnMap column out of bounds",DebugType::VERBOSE);
        return false;
    }
    if (globalSettings.currentEditingBackground < 1) {
        YUtils::printDebug("currentEditingBackground value in placeNewTileOnMap too low",DebugType::ERROR);
        YUtils::popupAlert("currentEditingBackground value in placeNewTileOnMap too low");
        return false;
    }
    if (globalSettings.currentEditingBackground > 3) {
        YUtils::printDebug("currentEditingBackground value in placeNewTileOnMap too high",DebugType::ERROR);
        YUtils::popupAlert("currentEditingBackground value in placeNewTileOnMap too high");
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
    auto scen = this->yidsRom->mapData->getScenByBg(globalSettings.currentEditingBackground,false);
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
    auto vramChartiles = scen->getVramChartiles();
    Chartile loadedTile;
    try {
        loadedTile = vramChartiles.at(mapRecord.tileId);
    } catch (...) {
        // 0 often just means "empty," but is not consistent
        // Use this as a fallback until you find out
        if (mapRecord.tileId != 0) {
            std::stringstream ssTile;
            ssTile << "Could not get certain tileId for BG" << globalSettings.currentEditingBackground << ": " << std::hex << mapRecord.tileId;
            YUtils::printDebug(ssTile.str(),DebugType::ERROR);
        } else {
            YUtils::printDebug("tileId in placeNewTileOnMap was 0",DebugType::WARNING);
        }
        return false;
    }
    auto pal = mapRecord.paletteId;
    pal += scen->paletteStartOffset - 1;
    auto isColorMode256 = scen->getInfo()->colorMode == BgColorMode::MODE_256;
    if (globalSettings.currentEditingBackground == 1) {
        // BG 1 //
        curItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
        if (!isColorMode256) {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->get256Palettes(pal+1));
        }
        curItem->setData(PixelDelegateData::PALETTE_ID_BG1,pal);
        curItem->setData(PixelDelegateData::FLIP_H_BG1,mapRecord.flipH);
        curItem->setData(PixelDelegateData::FLIP_V_BG1,mapRecord.flipV);
        curItem->setData(PixelDelegateData::TILEATTR_BG1,(uint)mapRecord.tileAttr);
        curItem->setData(PixelDelegateData::TILE_ID_BG1,(uint)mapRecord.tileId);
    } else if (globalSettings.currentEditingBackground == 2) {
        // BG 2 //
        curItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
        if (!isColorMode256) {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(pal+1));
        }
        curItem->setData(PixelDelegateData::PALETTE_ID_BG2,pal);
        curItem->setData(PixelDelegateData::FLIP_H_BG2,mapRecord.flipH);
        curItem->setData(PixelDelegateData::FLIP_V_BG2,mapRecord.flipV);
        curItem->setData(PixelDelegateData::TILEATTR_BG2,(uint)mapRecord.tileAttr);
        curItem->setData(PixelDelegateData::TILE_ID_BG2,(uint)mapRecord.tileId);
    } else if (globalSettings.currentEditingBackground == 3) {
        // BG 3 //
        curItem->setData(PixelDelegateData::PIXEL_ARRAY_BG3,loadedTile.tiles);
        if (!isColorMode256) {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
        } else {
            curItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->get256Palettes(pal+1));
        }
        curItem->setData(PixelDelegateData::PALETTE_ID_BG3,pal);
        curItem->setData(PixelDelegateData::FLIP_H_BG3,mapRecord.flipH);
        curItem->setData(PixelDelegateData::FLIP_V_BG3,mapRecord.flipV);
        curItem->setData(PixelDelegateData::TILEATTR_BG3,(uint)mapRecord.tileAttr);
        curItem->setData(PixelDelegateData::TILE_ID_BG3,(uint)mapRecord.tileId);
    } else {
        YUtils::printDebug("Unusual currentEditingBackground in placeNewTileOnMap",DebugType::ERROR);
        YUtils::popupAlert("Unusual currentEditingBackground in placeNewTileOnMap");
        return false;
    }
    curItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
    curItem->setData(PixelDelegateData::HOVER_TYPE,HoverType::NO_HOVER);
    // You are changing the MAP TILES not the pixels you dingus. VRAM is moot
    auto mpbzMaybe = scen->getFirstDataByMagic(Constants::MPBZ_MAGIC_NUM);
    auto mpbz = static_cast<MapTilesData*>(mpbzMaybe);
    uint32_t index = column + (row * scen->getInfo()->layerWidth);
    mpbz->tileRenderData.at(index) = mapRecord.compile();
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
            ssEmptyPreRender << bgIndex << " found";
            YUtils::printDebug(ssEmptyPreRender.str(),DebugType::WARNING);
            continue; // Proceed to next BG
        }
        BgColorMode colorMode = curScen->getInfo()->colorMode;
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
                MapTileRecordData curPren = YUtils::getMapTileRecordDataFromShort(mapTiles.at(preRenderIndex),colorMode);
                this->putTileBg(x,y,curPren,bgIndex);
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
        if (objectGraphicsMeta.tilesCount == 0) {
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
                topRight = new QTableWidgetItem();
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
    emit this->triggerMainWindowUpdate();
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
    auto objectPalette = this->yidsRom->backgroundPalettes[0]; // Default
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

    if (objectPalette.size() < 0xf) {
        std::stringstream ssEmptyPalette;
        ssEmptyPalette << "Palette size is too small! Size: 0x";
        ssEmptyPalette << std::hex << objectPalette.size();
        ssEmptyPalette << ", paletteFile value is 0x" << std::hex << paletteFile;
        ssEmptyPalette << ", object uuid is 0x" << std::hex << uuid;
        YUtils::printDebug(ssEmptyPalette.str(),DebugType::ERROR);
    }
    if (objectVector.size() == 0) {
        std::stringstream ssEmptyVector;
        ssEmptyVector << "Object vector size is too small! Size: 0x";
        ssEmptyVector << std::hex << objectPalette.size();
        ssEmptyVector << ", objectFile value is 0x" << std::hex << paletteFile;
        ssEmptyVector << ", object uuid is 0x" << std::hex << uuid;
        YUtils::printDebug(ssEmptyVector.str(),DebugType::ERROR);
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
            int16_t xOffset = YUtils::getSint16FromVec(objectVector, addrOfPositionRecord + 2); // Needs printf to show up:
            int16_t yOffset = YUtils::getSint16FromVec(objectVector, addrOfPositionRecord + 4); // printf("y: %d\n",yOffset);
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
                ssChunk << "Tried to get too big a chunk! Wanted " << std::hex << subEnd;
                ssChunk << ", only had " << std::hex << subLength;
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

void DisplayTable::wipeObject(uint32_t uuid) {
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
                potentialItem->setData(PixelDelegateData::OBJECT_UUID,QVariant::fromValue(nullptr));
                potentialItem->setData(PixelDelegateData::OBJECT_ID,QVariant::fromValue(nullptr));
                potentialItem->setData(PixelDelegateData::OBJECT_PALETTE,QVariant::fromValue(nullptr));
                potentialItem->setData(PixelDelegateData::OBJECT_TILES,QVariant::fromValue(nullptr));
            }
        }
    }
}
