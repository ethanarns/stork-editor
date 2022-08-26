#ifndef CHARTILESTRUCT_H
#define CHARTILESTRUCT_H

#include <cstdint>
#include <string>
#include <sstream>
#include <QByteArray>

enum ScreenEngine {
    A, // Top screen
    B  // Bottom screen
};

struct Chartile {
    ScreenEngine engine; // A (top screen) or B (bottom screen)
    uint32_t index;
    QByteArray tiles; // 8x8, each 0-15, representing their connection the current palette
    uint32_t offset;
};

struct ChartilePreRenderData {
    uint32_t tileId;
    uint8_t paletteId;
    bool flipH;
    bool flipV;
    uint16_t tileAttr;
    std::string toString() {
        std::stringstream ssChartilePreRend;
        ssChartilePreRend << "ChartilePreRenderData { tileId: " << std::hex << this->tileId;
        ssChartilePreRend << ", paletteId: " << std::hex << this->paletteId;
        ssChartilePreRend << ", flip H/V: " << this->flipH << "/" << this->flipV << " }";
        return ssChartilePreRend.str();
    }
};

#endif