#ifndef LEVELOBJECT_H
#define LEVELOBJECT_H

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
// setw()
#include <iomanip>

// std::function<>
#include <functional>

#include <QByteArray>

struct ObjectGraphicMetadata {
    ObjectGraphicMetadata() {
        this->isLz10 = false;
        this->is256 = false;
        this->frame = 0;
        this->xPixelOffset = 0;
        this->yPixelOffset = 0;
        this->forceFlipH = false;
        this->forceFlipV = false;
        this->extras = std::vector<ObjectGraphicMetadata>();
        this->whichObjectFile = "objset.arcz";
        this->whichPaletteFile = "objset.arcz";
        this->indexOfTiles = 0; // Use coin
        this->indexOfPalette = 0x99; // Look broken
    };
    /**
     * @brief The offset/index of the OBJB data.
     * This is hard-coded in the code sadly, no tables
     */
    uint32_t indexOfTiles;
    std::string whichPaletteFile;
    std::string whichObjectFile;
    /**
     * @brief The offset/index of the PLTB data.
     * This is hard-coded in the code sadly, no tables
     */
    uint32_t indexOfPalette;
    
    // The following are made obsolete once the size values were found //

    // /**
    //  * @brief How many tiles (8x8) wide this is before looping
    //  */
    // uint32_t tileWidth;
    // /**
    //  * @brief How many tiles will be printed out, not factoring in loop
    //  */
    // uint32_t tilesCount;

    bool isLz10;

    bool is256;

    uint32_t frame;

    /**
     * Sometimes the rounding is off. Use this to adjust the rounding enough to place
     * it in a better-looking tile
     */
    int32_t xPixelOffset;
    /**
     * Sometimes the rounding is off. Use this to adjust the rounding enough to place
     * it in a better-looking tile
     */
    int32_t yPixelOffset;

    bool forceFlipV;
    bool forceFlipH;

    std::vector<ObjectGraphicMetadata> extras;
};

struct ObjectTextMetadata {
    std::string prettyName;
    std::string description;
};

class LevelObject {
public:
    uint16_t objectId;
    uint16_t settingsLength;
    uint16_t xPosition;
    uint16_t yPosition;
    uint32_t uuid;
    std::vector<uint8_t> settings;
    static ObjectGraphicMetadata getObjectGraphicMetadata(LevelObject lo);

    std::string toString() {
        this->settingsLength = this->settings.size();
        std::stringstream ssLevelObject;
        ssLevelObject << "LevelObject {";
        ssLevelObject << " objectId: " << std::setw(4) << std::setfill('0') << std::hex << this->objectId;
        ssLevelObject << ", x/y: " << std::setw(4) << std::setfill('0') << std::hex << this->xPosition;
        ssLevelObject << "/" << std::setw(4) << std::setfill('0') << std::hex << this->yPosition;
        ssLevelObject << ", settingsLength: " << std::hex << this->settingsLength;
        ssLevelObject << ", uuid: " << std::hex << this->uuid;
        ssLevelObject << " }";
        return ssLevelObject.str();
    };
};
#endif