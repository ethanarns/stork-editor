#ifndef YIDSROM_H
#define YIDSROM_H

// std::string
#include <string>
// std::ifstream
#include <fstream>
// std::vector
#include <vector>
// std::map<key,value>
#include <map>

#include "Chartile.h"
#include "constants.h"
#include "LevelObject.h"

#include <QByteArray>

// Variable sizes and explanations: http://problemkaputt.de/gbatek.htm#dscartridgeheader
/**
 * @brief The YIDS ROM's Metadata. Mainly from the header.
 */
struct RomMetadata {
    uint32_t fatTableOffset;
    uint32_t fatTableSize;
    uint32_t fatOffsets;
    uint32_t fatSize;
};

class YidsRom {
public:
    const char* GAME_CODE = "AYWE";
    std::map<std::string,uint32_t> fileIdMap;
    std::fstream romFile;
    RomMetadata metadata;
    QByteArray currentPalettes[0x10]; // Will probably only use first 8
    /**
     * @brief "Characters", or the pixel arrangement of tiles
     */
    std::vector<Chartile> pixelTiles;
    /**
     * @brief BG 2's tile attr data, and the main source of level design. 
     * Corresponds with collision the most
     */
    std::vector<uint16_t> preRenderDataBg2;
    /**
     * @brief BG 1's tile data. Animated foreground for the most part, 
     * no collision (seen so far)
     */
    std::vector<uint16_t> preRenderDataBg1;
    /**
     * @brief Collision tiles, each taking up a full 16x16 tile. Also handles 
     * static normal coins for some reason
     */
    std::vector<uint8_t> collisionTileArray;

    std::vector<LevelObject> loadedLevelObjects;
    uint32_t canvasWidth = 0;
    uint32_t canvasHeight = 0;
    bool verbose;
    bool filesLoaded = false;
    YidsRom(bool verbose);
    void openRom(std::string fileName);

    std::string getLevelFileNameFromMapIndex(uint32_t worldIndex, uint32_t levelIndex);
    void loadCrsb(std::string fileName_noext);
    void wipeCrsbData();
    void getGameLevelsMetaData();

    std::string getTextAt(uint32_t position, uint32_t length);
    std::string getTextNullTermAt(uint32_t position_file);
    Address getAddrFromAddrPtr(Address pointerAddress_file);

    std::vector<uint8_t> getFileByteVector(std::string fileName);

    template<typename T>
    T getNumberAt(Address addr);

    ~YidsRom();
private:

    void initArm9RomData(std::string fileName);
    void writeUncompressedARM9(uint32_t arm9start_rom, uint32_t arm9length);
    void loadMpdz(std::string fileName_noext);
    void handleSCEN(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    void handleImbz(std::string fileName_noext);
    void handleGrad(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    void handleSETD(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
};

#endif