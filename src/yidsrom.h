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
    QByteArray currentPalettes[0x20];
    /**
     * @brief "Characters", or the pixel arrangement of tiles. From IMBZ data, for BG2
     */
    std::map<uint32_t,Chartile> pixelTilesBg2;
    uint32_t pixelTilesBg2index = 0;
    /**
     * @brief "Characters", or the pixel arrangement of tiles. From IMBZ data, for BG1
     */
    std::map<uint32_t,Chartile> pixelTilesBg1;
    uint32_t pixelTilesBg1index = 0;
    /**
     * @brief "Characters", or the pixel arrangement of tiles. From OBJ data, for OAM
     */
    std::map<uint32_t,std::vector<uint8_t>> objsetPixelTiles;
    //std::map<uint32_t,std::vector<ObjectDrawInstruction>> pixelTilesObj;
    
    /**
     * @brief BG 2's tile attr data, and the main source of level design. 
     * Corresponds with collision the most
     */
    std::vector<uint16_t> preRenderDataBg2;
    uint32_t colorModeBg2;
    /**
     * @brief BG 1's tile data. Animated foreground for the most part, 
     * no collision (seen so far)
     */
    std::vector<uint16_t> preRenderDataBg1;
    uint32_t colorModeBg1;
    /**
     * @brief BG 3's tile data. Animated bg?
     */
    std::vector<uint16_t> preRenderDataBg3;
    uint32_t colorModeBg3;
    /**
     * @brief Collision tiles, each taking up a full 16x16 tile. Also handles 
     * static normal coins for some reason
     */
    std::vector<uint8_t> collisionTileArray;

    std::map<uint32_t,ObjectPalette> objsetPalettes;

    std::map<uint32_t,std::vector<uint8_t>> effectPixelTiles;
    std::map<uint32_t,ObjectPalette> effectPalettes;
    std::map<uint32_t,ObjectFile> objectFiles;

    uint32_t paletteOffsetBg1 = 0;
    uint32_t paletteOffsetBg2 = 0;

    std::vector<LevelObject> loadedLevelObjects;
    uint32_t levelObjectLoadIndex = 0; // Instead of UUIDs

    uint32_t canvasWidthBg2 = 0;
    uint32_t canvasHeightBg2 = 0;
    uint32_t canvasWidthBg1 = 0;
    uint32_t canvasHeightBg1 = 0;
    uint32_t canvasWidthCol = 0;
    uint32_t canvasHeightCol = 0;
    bool filesLoaded = false;
    YidsRom();
    void openRom(std::string fileName);

    std::string getLevelFileNameFromMapIndex(uint32_t worldIndex, uint32_t levelIndex);
    CrsbData loadCrsb(std::string fileName_noext);
    void loadMpdz(std::string fileName_noext);
    void wipeCrsbData();
    void getGameLevelsMetaData();

    std::string getTextAt(uint32_t position, uint32_t length);
    std::string getTextNullTermAt(uint32_t position_file);
    Address getAddrFromAddrPtr(Address pointerAddress_file);

    std::vector<uint8_t> getFileByteVector(std::string fileName);

    template<typename T>
    T getNumberAt(Address addr){
        T container;
        this->romFile.seekg(addr);
        this->romFile.read(reinterpret_cast<char *>(&container), sizeof(container));
        return container;
    }

    uint32_t getGreatestCanvasWidth();
    uint32_t getGreatestCanvasHeight();

    ~YidsRom();
private:
    void initArm9RomData(std::string fileName);
    void writeUncompressedARM9(uint32_t arm9start_rom, uint32_t arm9length);
    ScenData handleSCEN(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    MpbzData handleMPBZ(std::vector<uint8_t>& uncompressedMpbz, uint16_t whichBg);
    ImbzData handleImbz(std::string fileName_noext, uint16_t whichBg);
    void handleGrad(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    SetdObjectData handleSETD(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    void handleAREA(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    PathData handlePATH(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    void handleALPH(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
    ObjectFile getMajorObjPltFile(std::string objset_filename, std::map<uint32_t,std::vector<uint8_t>>& pixelTiles, std::map<uint32_t,ObjectPalette>& palettes);
    ObjectFile getObjPltFile(std::string objset_filename);
};

#endif