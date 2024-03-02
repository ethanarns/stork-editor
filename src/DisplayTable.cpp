#include "DisplayTable.h"
#include "PixelDelegate.h"
#include "yidsrom.h"
#include "utils.h"

#include <QtCore>
#include <QTableWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QDrag>
#include <QApplication>

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

    QTableWidget::connect(this, &QTableWidget::cellEntered, this, &DisplayTable::cellEnteredTriggered);
    //QTableWidget::connect(this, &QTableWidget::cellClicked, this, &DisplayTable::displayTableClicked);
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

    // if (whichBg == 1) {
    //     pal += this->yidsRom->paletteOffsetBg1;
    // } else if (whichBg == 2) { ...
    pal += scen->paletteStartOffset - 1; // -1 is likely because of universal palette

    auto potentialExisting = this->item(y,x);
    auto isColorMode256 = scen->getInfo()->colorMode == BgColorMode::MODE_256;
    QByteArray layerDrawOrder = this->yidsRom->mapData->getLayerOrder();
    if (layerDrawOrder.size() > 3) {
        YUtils::printDebug("Size error in layer order, should be 3, was:",DebugType::FATAL);
        YUtils::printQbyte(layerDrawOrder);
        exit(EXIT_FAILURE);
    }
    if (potentialExisting == nullptr) {
        // Nothing is here, so lets make a new one and set it!
        QTableWidgetItem *newItem = new QTableWidgetItem();
        if (whichBg == 2) {
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
            if (!isColorMode256) {
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
            } else {
                // newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(this->yidsRom->paletteOffsetBg2 + 1));
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(pal+1));
            }
            newItem->setData(PixelDelegateData::FLIP_H_BG2,pren.flipH);
            newItem->setData(PixelDelegateData::FLIP_V_BG2,pren.flipV);
            newItem->setData(PixelDelegateData::TILEATTR_BG2,(uint)pren.tileAttr);
            newItem->setData(PixelDelegateData::TILE_ID_BG2,(uint)pren.tileId);
        } else if (whichBg == 1) {
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
            newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
            newItem->setData(PixelDelegateData::FLIP_H_BG1,pren.flipH);
            newItem->setData(PixelDelegateData::FLIP_V_BG1,pren.flipV);
            newItem->setData(PixelDelegateData::TILEATTR_BG1,(uint)pren.tileAttr);
            newItem->setData(PixelDelegateData::TILE_ID_BG1,(uint)pren.tileId);
        } else if (whichBg == 3) {
            newItem->setData(PixelDelegateData::PIXEL_ARRAY_BG3,loadedTile.tiles);
            if (!isColorMode256) {
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
            } else {
                // Was just 0xf before
                newItem->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->get256Palettes(pal+1));
            }
            newItem->setData(PixelDelegateData::FLIP_H_BG3,pren.flipH);
            newItem->setData(PixelDelegateData::FLIP_V_BG3,pren.flipV);
            newItem->setData(PixelDelegateData::TILEATTR_BG3,(uint)pren.tileAttr);
            newItem->setData(PixelDelegateData::TILE_ID_BG3,(uint)pren.tileId);
        }

        // Things to do for every layer:
        newItem->setData(PixelDelegateData::LAYER_DRAW_ORDER,layerDrawOrder);
        newItem->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
        newItem->setData(PixelDelegateData::HOVER_TYPE,HoverType::NO_HOVER);

        // Only doing collision here because there's no data for it, so create it
        newItem->setData(PixelDelegateData::COLLISION_DRAW,CollisionDraw::CLEAR);
        newItem->setData(PixelDelegateData::SHOW_COLLISION,this->shouldShowCollision);

        this->setItem(y,x,newItem);
    } else {
        // There is already an item here, lets just update it
        if (whichBg == 2) {
            potentialExisting->setData(PixelDelegateData::PIXEL_ARRAY_BG2,loadedTile.tiles);
            if (!isColorMode256) {
                potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
            } else {
                // Note: the 256 palettes thing does not always start at 0x10 (including the +1)
                // 1-3, there's a palette missing from the palette screen that made this start at 0xf
                // this will be moot if you separate the 256 palette
                potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG2,this->yidsRom->get256Palettes(pal+1));
            }
            potentialExisting->setData(PixelDelegateData::FLIP_H_BG2,pren.flipH);
            potentialExisting->setData(PixelDelegateData::FLIP_V_BG2,pren.flipV);
            potentialExisting->setData(PixelDelegateData::TILEATTR_BG2,(uint)pren.tileAttr);
            potentialExisting->setData(PixelDelegateData::TILE_ID_BG2,(uint)pren.tileId);
        } else if (whichBg == 1) {
            potentialExisting->setData(PixelDelegateData::PIXEL_ARRAY_BG1,loadedTile.tiles);
            potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG1,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
            potentialExisting->setData(PixelDelegateData::FLIP_H_BG1,pren.flipH);
            potentialExisting->setData(PixelDelegateData::FLIP_V_BG1,pren.flipV);
            potentialExisting->setData(PixelDelegateData::TILEATTR_BG1,(uint)pren.tileAttr);
            potentialExisting->setData(PixelDelegateData::TILE_ID_BG1,(uint)pren.tileId);
        } else if (whichBg == 3) {
            potentialExisting->setData(PixelDelegateData::PIXEL_ARRAY_BG3,loadedTile.tiles);
            if (!isColorMode256) {
                potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->mapData->getBackgroundPalettes(this->yidsRom->universalPalette).at(pal));
            } else {
                // Note: the 256 palettes thing does not always start at 0x10 (including the +1)
                // 1-3, there's a palette missing from the palette screen
                // this will be moot if you separate the 256 palette
                potentialExisting->setData(PixelDelegateData::PALETTE_ARRAY_BG3,this->yidsRom->get256Palettes(pal+1));
            }
            potentialExisting->setData(PixelDelegateData::FLIP_H_BG3,pren.flipH);
            potentialExisting->setData(PixelDelegateData::FLIP_V_BG3,pren.flipV);
            potentialExisting->setData(PixelDelegateData::TILEATTR_BG3,(uint)pren.tileAttr);
            potentialExisting->setData(PixelDelegateData::TILE_ID_BG3,(uint)pren.tileId);
        }
        // Things to do for every layer:
        potentialExisting->setData(PixelDelegateData::LAYER_DRAW_ORDER,layerDrawOrder);
        potentialExisting->setData(PixelDelegateData::DRAW_TRANS_TILES,false);
        potentialExisting->setData(PixelDelegateData::HOVER_TYPE,HoverType::NO_HOVER);
    }
}

void DisplayTable::cellEnteredTriggered(int y, int x) {
    QTableWidgetItem* curCell = this->item(y,x);
    if (curCell == nullptr) {
        return;
    } else {
        if (curCell->isSelected() && globalSettings.layerSelectMode == LayerMode::SPRITES_LAYER) {
            this->setCursor(Qt::OpenHandCursor);
        } else {
            this->setCursor(Qt::CustomCursor);
        }
        // BG Brush hovering
        this->updateSurrounding(y,x,20);
        curCell->setData(PixelDelegateData::HOVER_TYPE,HoverType::HOVER_SQUARE);
        // Top line
        for (int xTop = 1; xTop < globalSettings.brushW-1; xTop++) {
            this->setHover(y,x+xTop,HoverType::HOVER_TOP);
        }
        for (int yLeft = 1; yLeft < globalSettings.brushH-1; yLeft++) {
            this->setHover(y+yLeft,x,HoverType::HOVER_LEFT);
        }

        this->setHover(y                        ,x+globalSettings.brushW-1,HoverType::HOVER_TR);
        this->setHover(y+globalSettings.brushH-1,x+globalSettings.brushW-1,HoverType::HOVER_BR);
        this->setHover(y+globalSettings.brushH-1,x                        ,HoverType::HOVER_BL);
    }
}

void DisplayTable::printCellDebug(QTableWidgetItem *item, uint whichBg) {
    std::cout << "** Printing cell debug **" << std::endl;
    if (item == nullptr) {
        YUtils::printDebug("Item is null, can't print debug",DebugType::WARNING);
        return;
    }
    if (whichBg == 1) {

    } else if (whichBg == 2) {
        if (!item->data(PixelDelegateData::PIXEL_ARRAY_BG2).isNull()) {
            auto tileAttr = item->data(PixelDelegateData::TILEATTR_BG2).toUInt();
            YUtils::printDebug(YUtils::getCharPreRender(tileAttr).toString());
            auto pixArray2 = item->data(PixelDelegateData::PIXEL_ARRAY_BG2).toByteArray();
            YUtils::printDebug("Pixel Array for BG 2:",DebugType::VERBOSE);
            YUtils::printQbyte(pixArray2);
            YUtils::printDebug("Palette for BG2:",DebugType::VERBOSE);
            auto pal = item->data(PixelDelegateData::PALETTE_ARRAY_BG2).toByteArray();
            YUtils::printQbyte(pal);
        }
    } else if (whichBg == 3) {

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
                this->clearSelection();
                this->selectedObjects.clear();
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
                this->clearSelection();
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
                this->clearSelection();
                this->selectedObjects.clear();

                YUtils::printDebug("Doing item selection in mousePressEvent");
                this->selectItemByUuid(cursorItemUuid);
                return;
            } else {
                YUtils::printDebug("Area clicked does not have an item UUID, deselecting all");
                this->clearSelection();
                this->selectedObjects.clear();
                return;
            }
        }
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
        return;
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
        return;
    }
    QTableWidget::mousePressEvent(event);
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

void DisplayTable::selectItemByUuid(uint32_t uuid) {
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
                potentialItem->setSelected(true);
            }
        }
    }
    emit this->triggerMainWindowUpdate();
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
    auto collisionTileArray = this->yidsRom->mapData->getCollisionArray();
    const int CELL_LIST_SIZE = collisionTileArray.size();
    if (CELL_LIST_SIZE < 1) {
        YUtils::printDebug("Collision Tile Array is empty!",DebugType::ERROR);
        return;
    }
    for (int colIndex = 0; colIndex < CELL_LIST_SIZE; colIndex++) {
        const auto curCol = collisionTileArray.at(colIndex);
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
        YUtils::printDebug("No TriggerBoxes (AREA) for this map",DebugType::VERBOSE);
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
    this->clearSelection();
    this->selectedObjects.clear();
    this->selectItemByUuid(uuid);
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
                ChartilePreRenderData curPren = YUtils::getCharPreRender(mapTiles.at(preRenderIndex),colorMode);
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
        auto objectTextMeta = LevelObject::getObjectTextMetadata(it->objectId);
        if (objectGraphicsMeta.tilesCount == 0) {
            // top left I think
            potentialExisting->setData(PixelDelegateData::OBJECT_ID,(uint32_t)it->objectId);
            potentialExisting->setData(PixelDelegateData::OBJECT_UUID,it->uuid);
            potentialExisting->setText("sprite");
            potentialExisting->setData(PixelDelegateData::OBJECT_PALETTE,this->yidsRom->backgroundPalettes[0]);
            std::stringstream ss;
            ss << "0x" << std::hex << (uint32_t)it->objectId;
            ss << std::endl;
            ss << objectTextMeta.prettyName;
            ss << std::endl;
            ss << objectTextMeta.description;
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
