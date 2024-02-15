#ifndef CHARTILESTRUCT_H
#define CHARTILESTRUCT_H

#include <cstdint>
#include <string>
#include <sstream>
#include <QByteArray>

enum BgColorMode {
    MODE_16 = 0,
    MODE_256 = 1
};

struct Chartile {
    /// @brief Either 16 or 256
    BgColorMode bgColMode;
    /// @brief This is for debug purposes only, showing the index where the tile was loaded in VRAM
    uint32_t index;
    /// @brief 64 tiles representing 8x8 square of palette color values
    /// @note If in 16 bit color mode, each are 0-15. 256 bit mode, 0-255
    QByteArray tiles;
};

/// @brief This is a breaking-up of the 2 byte (or 1 byte in 256 color mode) MPBZ data records
/// @note http://problemkaputt.de/gbatek.htm#lcdvrambgscreendataformatbgmap
struct ChartilePreRenderData {
    /// @brief Points to a certain Chartile in the VRAM
    uint32_t tileId;
    /// @brief Which color palette to use when rendering the tile
    uint8_t paletteId;
    bool flipH;
    bool flipV;
    
    /// @brief For debugging. It's the OG value
    uint16_t tileAttr;
    std::string toString() {
        std::stringstream ssChartilePreRend;
        ssChartilePreRend << "ChartilePreRenderData { tileId: " << std::hex << this->tileId;
        ssChartilePreRend << ", paletteId: " << std::hex << (uint16_t)this->paletteId;
        ssChartilePreRend << ", flip H/V: " << this->flipH << "/" << this->flipV << " }";
        return ssChartilePreRend.str();
    }
};

#endif