#ifndef LEVELOBJECT_H
#define LEVELOBJECT_H

#include <cstdint>
#include <string>
#include <vector>

#include <QByteArray>

enum PaletteFileName {
    OBJSET,
    OBJEFFECT
};

struct ObjectGraphicMetadata {
    /**
     * @brief The offset/index of the OBJB data.
     * This is hard-coded in the code sadly, no tables
     */
    uint32_t tilesSectorOffset;
    PaletteFileName whichPaletteFile;
    /**
     * @brief The offset/index of the PLTB data.
     * This is hard-coded in the code sadly, no tables
     */
    uint32_t paletteSectorOffset;
    /**
     * @brief How many tiles (8x8) wide this is before looping
     */
    uint32_t tileWidth;
    /**
     * @brief How many tiles will be printed out, not factoring in loop
     */
    uint32_t tilesCount;

    uint32_t subTile;
};

class LevelObject {
public:
    uint16_t objectId;
    uint16_t settingsLength;
    uint16_t xPosition;
    uint16_t yPosition;
    std::vector<uint8_t> settings;
    static ObjectGraphicMetadata getObjectGraphicMetadata(uint16_t objectId);
};
#endif