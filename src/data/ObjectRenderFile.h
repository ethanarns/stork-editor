#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <sstream>

struct ObjbFrame {
    uint16_t buildOffset;
    uint8_t holdTime;
    int8_t frameJump;
    std::string toString() {
        std::stringstream ss;
        ss << "ObjbFrame { buildOffset: 0x";
        ss << std::hex << this->buildOffset;
        ss << ", holdTime: 0x" << std::hex << (uint16_t)this->holdTime;
        ss << ", frameJump: " << std::dec << (int16_t)this->frameJump;
        ss << " }";
        return ss.str();
    };
};

struct ObjFrameBuild {
    uint16_t tileOffset;
    int16_t xOffset;
    int16_t yOffset;
    uint16_t flags;
    std::string toString() {
        std::stringstream ss;
        ss << "ObjbFrameBuild { tileOffset: 0x";
        ss << std::hex << this->tileOffset;
        ss << ", xOffset: " << std::dec << this->xOffset;
        ss << ", yOffset: " << std::dec << this->yOffset;
        ss << ", flags: 0x" << std::hex << this->flags;
        ss << " }";
        return ss.str();
    };
};

// OBJB/OBJZ record
class ObjectTileData {
public:
    // Uncompressed
    ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t length);

    std::vector<ObjbFrame*> frames;
    std::vector<ObjFrameBuild*> frameBuilds;
};

// OBAR
class ObjectRenderArchive {
public:
    ObjectRenderArchive(std::vector<uint8_t> obarVector);
};