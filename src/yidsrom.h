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
    bool verbose;
    YidsRom(bool verbose);
    void openRom(std::string fileName);

    std::string getLevelFileNameFromMapIndex(uint32_t levelIndex, uint32_t worldIndex);

    std::string getTextAt(uint32_t position, uint32_t length);
    std::string getTextNullTermAt(uint32_t position_file);
    uint32_t conv2xAddrToFileAddr(uint32_t x2address);
    uint32_t getAddrFromAddrPtr(uint32_t fileAddress);

    std::vector<uint8_t> getFileByteVector(std::string fileName);

    template<typename T>
    T getNumberAt(uint32_t addr);

    ~YidsRom();
private:
    bool filesLoaded = false;

    void initArm9RomData(std::string fileName);
    void writeUncompressedARM9(uint32_t arm9start_rom, uint32_t arm9length);
    void loadCrsb(std::string fileName_noext);
    void loadMpdz(std::string fileName_noext);
    void handleSCEN(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer);
};

#endif