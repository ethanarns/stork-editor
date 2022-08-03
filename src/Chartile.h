#ifndef CHARTILESTRUCT_H
#define CHARTILESTRUCT_H

#include <cstdint>
#include <QByteArray>

enum ScreenEngine {
    A, // Top screen
    B  // Bottom screen
};

struct Chartile {
    ScreenEngine engine; // A (top screen) or B (bottom screen)
    uint32_t index;
    QByteArray tiles; // 8x8, each 0-15, representing their connection the current palette
};

struct ChartilePreRenderData {
    uint32_t tileId;
    uint8_t paletteId;
    bool flipH;
    bool flipV;
    uint16_t tileAttr;
};

#endif