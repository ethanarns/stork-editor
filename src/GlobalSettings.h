#ifndef SETTINGSENUM_H
#define SETTINGSENUM_H

#include <cstdint>
#include <vector>

enum LayerMode {
    BG1_LAYER,
    BG2_LAYER,
    BG3_LAYER,
    SPRITES_LAYER,
    COLLISION_LAYER
};

struct TileBrush {
    std::vector<uint32_t> tileAttrs;
    const static int BRUSH_DIMS = 16;
};

struct GlobalSettings {
    TileBrush* currentBrush;
    std::vector<TileBrush*> brushes;
    uint32_t currentTileIndex = 0xffff; // Only can go up to about 1024 tiles
    uint32_t currentPaletteIndex = 0;
    uint32_t currentEditingBackground = 0; // 0 = not a bg
    LayerMode layerSelectMode;
};
extern GlobalSettings globalSettings;

#endif