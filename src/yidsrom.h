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
#include "PixelDelegate.h"
#include "Level.h"

#include "data/MapData.h"

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
    std::vector<uint8_t> uncompedRomVector;
    RomMetadata metadata;
    MapData* mapData;

    QByteArray universalPalette;

    QByteArray backgroundPalettes[0x20];

    /**
     * @brief "Characters", or the pixel arrangement of tiles. From OBJ data, for OAM
     */
    std::map<uint32_t,std::vector<uint8_t>> objsetPixelTiles;

    std::map<uint32_t,ObjectPalette> objsetPalettes;

    std::map<uint32_t,std::vector<uint8_t>> effectPixelTiles;
    std::map<uint32_t,ObjectPalette> effectPalettes;
    std::map<uint32_t,ObjectFile> objectFiles;

    bool filesLoaded = false;
    YidsRom();
    void openRom(std::string fileName);

    std::string getLevelFileNameFromMapIndex(uint32_t worldIndex, uint32_t levelIndex);
    //CrsbData loadCrsb(std::string fileName_noext);
    void loadMpdz(std::string fileName_noext);
    void wipeLevelData();
    QByteArray get256Palettes(uint32_t offset);

    void moveObject(uint32_t objectUuid, int xOffset, int yOffset);
    void moveObjectTo(uint32_t objectUuid, uint32_t newX, uint32_t newY);

    //std::string getTextAt(uint32_t position, uint32_t length);
    //std::string getTextNullTermAt(uint32_t position_file);
    Address getAddrFromAddrPtr(Address pointerAddress_file);

    std::vector<uint8_t> getByteVectorFromFile(std::string fileName);

    // template<typename T>
    // T getNumberAt(Address addr){
    //     T container;
    //     this->romFile.seekg(addr);
    //     this->romFile.read(reinterpret_cast<char *>(&container), sizeof(container));
    //     return container;
    // }

    ~YidsRom();
private:
    void initArm9RomData(std::string fileName, std::vector<uint8_t> &compedRom);
    ObjectFile getMajorObjPltFile(std::string objset_filename, std::map<uint32_t,std::vector<uint8_t>>& pixelTiles, std::map<uint32_t,ObjectPalette>& palettes);
    ObjectFile getObjPltFile(std::string objset_filename);
};

#endif