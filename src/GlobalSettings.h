#ifndef SETTINGSENUM_H
#define SETTINGSENUM_H

#include <cstdint>
#include <vector>
#include <QTableWidgetItem>

#include "PixelDelegateEnums.h"

enum LayerMode {
    BG1_LAYER,
    BG2_LAYER,
    BG3_LAYER,
    SPRITES_LAYER,
    COLLISION_LAYER
};

struct TileBrush {
    std::vector<MapTileRecordData> tileAttrs;
    const static int BRUSH_DIMS = 16;
};

struct GlobalSettings {
    TileBrush* currentBrush;
    std::vector<TileBrush*> brushes;
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