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
#include "PixelDelegateEnums.h"
#include "Level.h"

#include "data/MapData.h"
#include "data/LevelSelectData.h"
#include "data/ObjectRenderFile.h"

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

/**
 * Everything needed to place and get info on a Sprite
 * Doesn't contain much render data
*/
struct SpriteMeta {
    uint16_t spriteId;
    std::string name;
    std::string info;
    // ONLY use this for creating the object itself
    uint16_t createdSettingsLen;
};

class YidsRom {
public:
    const char* GAME_CODE = "AYWE";

    std::vector<uint8_t> uncompedRomVector;
    RomMetadata metadata;
    // MPDZ
    MapData* mapData;
    // CRSB
    LevelSelectData* currentLevelSelectData;

    std::vector<SpriteMeta> spriteMetadata;
    SpriteMeta getSpriteMetadata(uint32_t spriteId);

    QByteArray universalPalette;

    QByteArray backgroundPalettes[0x20];

    std::map<uint8_t,std::map<uint32_t, Chartile>> chartileVram;
    std::map<uint8_t,uint8_t> chartileVramPaletteOffset; // Linked with above, visual only
    void reloadChartileVram(uint frame = 0);

    std::map<std::string,ObjectFile> objectRenderFiles;
    std::map<std::string,ObjectRenderArchive*> spriteRenderFiles;

    bool filesLoaded = false;
    YidsRom();
    void openRom(std::string fileName);

    std::string getLevelFileNameFromMapIndex(uint32_t worldIndex, uint32_t levelIndex);
    //CrsbData loadCrsb(std::string fileName_noext);
    void loadMpdz(std::string fileName_noext);
    void wipeLevelData();

    void moveObject(uint32_t objectUuid, int xOffset, int yOffset);
    void moveObjectTo(uint32_t objectUuid, uint32_t newX, uint32_t newY);

    //std::string getTextAt(uint32_t position, uint32_t length);
    //std::string getTextNullTermAt(uint32_t position_file);
    Address getAddrFromAddrPtr(Address pointerAddress_file);

    std::vector<uint8_t> getByteVectorFromFile(std::string fileName);
    ObjectFile getObjPltFile(std::string objset_filename);
    bool loadObjectRenderFile(std::string obarFileFull);
    bool loadSpriteRenderFile(std::string obarFilenameFull);

    void getHintMessageData(uint16_t hintMessageId);

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

    void updateSpriteMeta();
};

#endif