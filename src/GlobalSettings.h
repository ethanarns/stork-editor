#ifndef SETTINGSENUM_H
#define SETTINGSENUM_H

#include <cstdint>
#include <string>
#include <vector>

#include <QTableWidgetItem>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include "PixelDelegateEnums.h"
#include "Chartile.h"

enum LayerMode {
    BG1_LAYER,
    BG2_LAYER,
    BG3_LAYER,
    SPRITES_LAYER,
    COLLISION_LAYER,
    PORTALS_LAYER
};

struct TileBrush {
    std::vector<MapTileRecordData> tileAttrs;
    std::string brushTileset;
    std::string name;
    QJsonObject toJson() {
        QJsonArray tileArray;
        for (auto it = this->tileAttrs.begin(); it != this->tileAttrs.end(); it++) {
            // All JSON numbers are signed
            QJsonValue tileAttrDouble = (int)(it->compile());
            tileArray.append(tileAttrDouble);
        }
        QJsonValue jstring = this->brushTileset.c_str();
        QJsonObject jobj;
        jobj["tileAttrs"] = tileArray;
        jobj["brushTileset"] = jstring;
        return jobj;
    };
};

struct GlobalSettings {
    TileBrush* currentBrush;
    std::vector<TileBrush> brushes;
    uint32_t currentTileIndex = 0xffff; // Only can go up to about 1024 tiles
    uint32_t currentPaletteIndex = 0;
    uint32_t currentEditingBackground = 0; // 0 = not a bg
    bool brushFlipH = false;
    bool brushFlipV = false;
    int brushW = 2;
    int brushH = 2;
    LayerMode layerSelectMode;
    CollisionType colTypeToDraw = CollisionType::SQUARE;
    std::vector<QTableWidgetItem*> selectedItemPointers;
    uint32_t currentSpriteIdToAdd = 0;
    uint32_t gridCellSizePx = 8;
    std::string extractFolderName = "_nds_unpack";

    uint32_t temp_paletteOffset = 0;

    int getSelectionWidth() {
        int leftMost = 0xffff;
        int rightMost = 0x0000;
        for (auto it = this->selectedItemPointers.begin(); it != this->selectedItemPointers.end(); it++) {
            auto item = *it;
            int curCol = item->column();
            if (curCol < leftMost) {
                leftMost = curCol;
            }
            if (curCol > rightMost) {
                rightMost = curCol;
            }
        }
        // A difference of one means there are 2 tiles selected
        int diff = rightMost - leftMost + 1;
        if (diff < 1) {
            return 0;
        } else {
            return diff;
        }
    };
};
extern GlobalSettings globalSettings;

#endif