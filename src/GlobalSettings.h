#ifndef SETTINGSENUM_H
#define SETTINGSENUM_H

#include <cstdint>
#include <vector>

struct TileBrush {
    std::vector<uint32_t> tileAttrs;
    const static int BRUSH_DIMS = 16;
};

struct GlobalSettings {
    TileBrush* currentBrush;
    std::vector<TileBrush*> brushes;
    uint32_t currentTileIndex = 0xffff; // Only can go up to about 1024 tiles
};
extern GlobalSettings globalSettings;

#endif