#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <sstream>

#include <QByteArray>

struct ObjFrameBuild {
    // Offset (<< 4) from very start of binary (or section?) to the frame in question
    // For example, 0xe << 0xe0, means the chartile data starts 0xe0 after
    uint16_t tileOffset;
    int16_t xOffset;
    int16_t yOffset;
    uint16_t flags;
    uint32_t _binOffset;
    std::string toString() {
        std::stringstream ss;
        ss << "ObjbFrameBuild { tileOffset: 0x";
        ss << std::hex << this->tileOffset;
        ss << ", xOffset: " << std::dec << this->xOffset;
        ss << ", yOffset: " << std::dec << this->yOffset;
        ss << ", flags: 0x" << std::hex << this->flags;
        ss << ", _binOffset: 0x" << std::hex << this->_binOffset;
        ss << " }";
        return ss.str();
    };
};

struct ObjPltb {
    std::vector<QByteArray> palettes;
    uint32_t _globalIndex;
    uint32_t _obarAddress;
};

struct ObjbFrame {
    // In the binary, this is a direct offset in bytes to the ObjFrameBuild associated, no <<s
    // Starts from first build offset, not OBJB or length
    uint16_t buildOffset;
    uint8_t holdTime;
    int8_t frameJump;
    ObjFrameBuild *buildFrame;
    uint32_t _binOffset;
    std::string toString() {
        std::stringstream ss;
        ss << "ObjbFrame { buildOffset: 0x";
        ss << std::hex << this->buildOffset;
        ss << ", holdTime: 0x" << std::hex << (uint16_t)this->holdTime;
        ss << ", frameJump: " << std::dec << (int16_t)this->frameJump;
        ss << ", _binOffset: 0x" << std::hex << this->_binOffset;
        ss << " }";
        return ss.str();
    };
};

// OBJB/OBJZ record
class ObjectTileData {
public:
    // Uncompressed
    ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t end);
    ObjectTileData(std::vector<uint8_t> decompVector);
    //ObjbFrame* getFrameData(uint32_t frameIndex);
    ObjbFrame getFrameAt(uint32_t frameIndex);
    std::vector<QByteArray> getChartiles(uint32_t index, uint32_t count);

    //std::vector<ObjbFrame*> frames;

    uint32_t _globalIndex;
    uint32_t _obarAddress;
private:
    // Store raw, as the program accesses it that way
    std::vector<uint8_t> byteData;
};

// OBAR
class ObjectRenderArchive {
public:
    ObjectRenderArchive(std::vector<uint8_t> obarVector);
    std::vector<ObjectTileData*> objectTileDataVector;
    std::vector<ObjPltb*> objectPaletteDataVector;
};