#include "yidsrom.h"
#include "utils.h"
#include "compression.h"
#include "Chartile.h"
#include "constants.h"

// std::cerr, std::endl, std::ios
#include <iostream>
// std::stringstream
#include <sstream>
// std::ifstream
#include <fstream>
// exit(), EXIT_FAILURE
#include <cstdlib>

// std::vector
#include <vector>

// std::tolower
#include <cctype>

#include <filesystem>

// Q_UNUSED
#include <QtGlobal>
#include <QByteArray>

YidsRom::YidsRom() {
    this->filesLoaded = false;
    this->preRenderDataBg2.reserve(180'000); // Found 189280 in 1-1's first IMBZ
    //this->collisionTileArray.reserve(79'000); // Found roughly 79000 in 1-1's first IMBZ
}

void YidsRom::openRom(std::string fileName) {
    this->romFile.open(fileName, ios::binary | ios::in);
    if (!romFile) {
        std::stringstream ssNoRom;
        ssNoRom << "ERROR: ROM file could not be opened: '" << fileName << "'. ";
        ssNoRom << "Reported load error code: " << errno;
        YUtils::printDebug(ssNoRom.str(),DebugType::FATAL);
        return;
    }
    this->filesLoaded = false;

    /************************
     *** METADATA/HEADERS ***
     ************************/
    std::string romCode = this->getTextAt(0x0c,4);
    if (romCode.compare(YidsRom::GAME_CODE) != 0) {
        std::stringstream ssGameCode;
        ssGameCode << "Game code AYWE not found! Got '" << romCode << "' instead. Wrong ROM?";
        YUtils::printDebug(ssGameCode.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    // http://problemkaputt.de/gbatek.htm#dscartridgeheader
    // NDSTool can check these (first 4 checked, correct)
    this->metadata.fatTableOffset = this->getNumberAt<uint32_t>(0x40);
    this->metadata.fatTableSize = this->getNumberAt<uint32_t>(0x44);
    this->metadata.fatOffsets = this->getNumberAt<uint32_t>(0x48);
    this->metadata.fatSize = this->getNumberAt<uint32_t>(0x4c);

    // Decompress the ARM9 ROM
    this->initArm9RomData(fileName);

    /*************************
     *** FILE MANIPULATION ***
     *************************/

    // http://problemkaputt.de/gbatek.htm#dscartridgenitroromandnitroarcfilesystems
    uint32_t offsetToFirstSubTable = this->getNumberAt<uint32_t>(this->metadata.fatTableOffset + 0);
    Address fileSubDirectory = offsetToFirstSubTable + this->metadata.fatTableOffset;
    uint8_t subdirTypeAndNameLength = this->getNumberAt<uint8_t>(fileSubDirectory);
    uint8_t subdirNameLength = subdirTypeAndNameLength % 0x10;
    // +1 because the text name starts at 1
    auto subdirName = this->getTextAt(fileSubDirectory + 1,subdirNameLength);
    const std::string SUBDIR_NAME = "file";
    if (subdirName.compare(SUBDIR_NAME) != 0) {
        std::stringstream ssFailSub;
        ssFailSub << "[FAIL] First subdirectory '" << SUBDIR_NAME << "' was not found! Found '" << subdirName << "' instead.";
        YUtils::printDebug(ssFailSub.str(), DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    // +4 because 4 bytes between end of text and first entry
    Address FILESDIR_BASE_ADDR = fileSubDirectory + (uint32_t)subdirNameLength + 4;

    const uint32_t FIRST_FILE_ID = 45; // 2D

    const Address FAT_TABLE_END = this->metadata.fatTableOffset + this->metadata.fatTableSize - 1; // Sub 1 to prevent blank end
    uint32_t fileOffset = 0;
    uint32_t curFileId = FIRST_FILE_ID;
    while (FILESDIR_BASE_ADDR + fileOffset < FAT_TABLE_END) {
        uint8_t len = this->getNumberAt<uint8_t>(FILESDIR_BASE_ADDR + fileOffset);
        if (len > 0x20) {
            std::stringstream ssLongLen;
            ssLongLen << "Long length: " << hex << (int)len << " at " << hex << (FILESDIR_BASE_ADDR + fileOffset);
            YUtils::printDebug(ssLongLen.str(),DebugType::WARNING);
        }
        auto nameStr = this->getTextAt(FILESDIR_BASE_ADDR + fileOffset + 1,len);
        // When finding files, it lowercases
        this->fileIdMap[YUtils::getLowercase(nameStr)] = curFileId;
        fileOffset += len + 1; // +1 accounts for length byte
        curFileId++;
    }

    auto objsetFile = this->getMajorObjPltFile("objset.arcz",this->objsetPixelTiles,this->objsetPalettes);
    auto effectFile = this->getMajorObjPltFile("objeffect.arcz",this->effectPixelTiles,this->effectPalettes);
    Q_UNUSED(objsetFile);
    Q_UNUSED(effectFile);

    this->objectFiles[ObjectFileName::OBJSBBLOCK] = this->getObjPltFile("objsbblock.arc");

    std::string crsbFileName = this->getLevelFileNameFromMapIndex(0,0);
    auto crsb = this->loadCrsb(crsbFileName);
    this->loadMpdz(crsb.cscnList.at(0).mpdzFileNoExtension);
}

std::string YidsRom::getTextAt(Address position_file, uint32_t length) {
    this->romFile.seekg(position_file);
    char* readChars = new char[length];
    this->romFile.read(readChars,length);
    std::string ret = readChars;
    // Cut out garbage data
    ret = ret.substr(0,length);
    delete[] readChars;
    return ret;
}

std::string YidsRom::getTextNullTermAt(Address position_file) {
    uint8_t offset = 0;
    constexpr uint8_t MAX_STRING_LENGTH = 0xff;
    uint8_t killOffset = 0;
    std::string result = "";

    this->romFile.seekg(position_file + offset);
    char container;
    const uint8_t NULL_TERM = 0x00;
    this->romFile.read(&container, sizeof(container));
    if (container == NULL_TERM) {
        std::stringstream nullTermSs;
        nullTermSs << "Found empty string at position " << hex << position_file;
        YUtils::printDebug(nullTermSs.str(),DebugType::ERROR);
        return result;
    }
    // Increment then return offset. 2 birds, meet 1 stone
    while (killOffset < MAX_STRING_LENGTH) {
        // ++ afterwards returns the original value THEN increments
        this->romFile.seekg(position_file + offset++);
        this->romFile.read(&container, sizeof(container));
        if (container == NULL_TERM) {
            return result;
        }
        result += container;
        killOffset++;
    }
    return "STRING LONGER THAN 0xFF";
}

YidsRom::~YidsRom() {
    // delete/delete[] things here
}

void YidsRom::initArm9RomData(std::string fileName) {
    // Start of ARM9 ROM data, aka "MainRomOffset"
    this->romFile.seekg(0x20);
    Address romStart9;
    this->romFile.read(reinterpret_cast<char *>(&romStart9),sizeof(romStart9));

    if (Constants::ARM9_ROM_OFFSET != romStart9) {
        std::stringstream ssRomOffset;
        ssRomOffset << "Found ARM9 ROM offset not equal to ";
        ssRomOffset << hex << Constants::ARM9_ROM_OFFSET;
        YUtils::printDebug(ssRomOffset.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    // Size of ARM9 ROM data, aka "MainSize"
    this->romFile.seekg(0x2c);
    uint32_t romSize9;
    this->romFile.read(reinterpret_cast<char *>(&romSize9),sizeof(romSize9));

    // End of ARM9 ROM (exclusive)
    Address endAddress = romStart9 + romSize9;

    this->romFile.seekg(endAddress);
    uint32_t endAddressMagicNumber;
    this->romFile.read(reinterpret_cast<char *>(&endAddressMagicNumber),sizeof(endAddressMagicNumber));
    if (endAddressMagicNumber != Constants::SDK_NITROCODE_BE) {
        std::stringstream ssMn;
        ssMn << "SDK_NITROCODE_BG magic number '" << hex << Constants::SDK_NITROCODE_BE << "' not found. ";
        ssMn << "Instead found " << hex << endAddressMagicNumber;
        YUtils::printDebug(ssMn.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    this->extractCompressedARM9(romStart9,romSize9);

    bool arm9decompResult = YCompression::blzDecompress(Constants::NEW_BIN_FILE, false);
    if (!arm9decompResult) {
        YUtils::printDebug("Could not decompress the ARM9 BIN!",DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    std::ifstream arm9fileUncomped;
    arm9fileUncomped.open(Constants::NEW_BIN_FILE, ios::binary);

    // Get arm9fileUncomped's length
    arm9fileUncomped.seekg(0, ios_base::end);
    size_t arm9fileLength = arm9fileUncomped.tellg();
    arm9fileUncomped.seekg(0, ios_base::beg);
    if (arm9fileLength == 0) {
        YUtils::printDebug("ARM9 binary empty!", DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    // Create buffer to hold arm9fileUncomped
    std::vector<uint8_t> arm9buffer;
    arm9buffer.reserve(arm9fileLength); // Prevent constant resizing
    std::copy(
        std::istreambuf_iterator<char>(arm9fileUncomped),
        std::istreambuf_iterator<char>(),
        std::back_inserter(arm9buffer)
    );

    // Copy existing file (okay to overwrite, edited rom is written to new file)
    std::filesystem::copy(
        fileName, Constants::NEW_ROM_FILE,
        std::filesystem::copy_options::overwrite_existing
    );

    // Change romFile to new one
    this->romFile.close();
    this->romFile.open(Constants::NEW_ROM_FILE, ios::binary | ios::in | ios::out);
    this->romFile.seekp(Constants::ARM9_ROM_OFFSET + 0x4000);
    this->romFile.write(reinterpret_cast<char *>(arm9buffer.data()),arm9fileLength);
    arm9fileUncomped.close();

    // Test it
    const Address TEST_POSITION = 0xe9e6e;
    const auto TEST_TEXT = "1-1_D3";
    auto testText1 = this->getTextAt(TEST_POSITION,6);
    if (testText1.compare(TEST_TEXT) != 0) {
        std::stringstream ssTestText1;
        ssTestText1 << "Test looked for '" << TEST_TEXT << "' with getTextAt, found '" << testText1 << "'";
        YUtils::printDebug(ssTestText1.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    auto testText2 = this->getTextNullTermAt(TEST_POSITION);
    if (testText2.compare(TEST_TEXT) != 0) {
        std::stringstream ssTestText2;
        ssTestText2 << "Test looked for '" << TEST_TEXT << "' with getTextNullTermAt, found '" << testText2 << "'";
        YUtils::printDebug(ssTestText2.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    // Delete the old file (Consider streaming to memory instead)
    filesystem::remove(Constants::NEW_ROM_FILE);

    // It worked! Report them as loaded successfully
    this->filesLoaded = true;

    // Write universal palette
    this->currentPalettes[0].resize(Constants::PALETTE_SIZE);
    Address universalPalette0base = YUtils::conv2xAddrToFileAddr(Constants::UNIVERSAL_PALETTE_0_ADDR);
    for (int univPalIndex = 0; univPalIndex < Constants::PALETTE_SIZE; univPalIndex++) {
        this->romFile.seekg(universalPalette0base + univPalIndex);
        uint8_t container;
        this->romFile.read(reinterpret_cast<char *>(&container), sizeof(container));
        this->currentPalettes[0][univPalIndex] = container;
    }
}

void YidsRom::extractCompressedARM9(Address arm9start_rom, uint32_t arm9length) {
    if (this->filesLoaded) {
        cout << "ARM9 has already been written and read" << endl;
        return;
    }

    std::vector<uint8_t> outvec(arm9length,0xfe);
    for (uint32_t arm9copyindexoffset = 0; arm9copyindexoffset < arm9length; arm9copyindexoffset++) {
        Address romLocation = arm9copyindexoffset + arm9start_rom;
        // Don't use this->getNumberAt yet, not loaded
        this->romFile.seekg(romLocation);
        uint8_t ret;
        this->romFile.read(reinterpret_cast<char *>(&ret),sizeof(ret));
        outvec.at(arm9copyindexoffset) = ret;
    }

    YUtils::writeByteVectorToFile(outvec,Constants::NEW_BIN_FILE);
}

/**
 * @brief Get the name for the level file, which is almost always a CRSB
 * 
 * @todo See if this ever gets used for non-CRSB files
 * 
 * @param worldIndex 0 inclusive world index, goes up to 4 (world 5 is last world)
 * @param levelIndex 0 inclusive level index, goes up to 9 (10 levels per world)
 * @return File name for the level, without extension
 */
std::string YidsRom::getLevelFileNameFromMapIndex(uint32_t worldIndex, uint32_t levelIndex) {
    if (worldIndex + 1 > 5) {
        cerr << "ERROR: World 5 is the highest world level there is" << endl;
        return "";
    }
    if (levelIndex + 1 > 10) {
        cerr << "ERROR: There are only 10 levels per world" << endl;
        return "";
    }
    const uint32_t levelId = worldIndex * 10 + levelIndex + 1;
    if (levelId < 0x7b || levelId > 0x7e) {
        // 02050024 (some function that takes in 0), does not break
    }
    if (levelId == 0) {
        // Hard set
        return "0-1_D3";
    } else {
        // 02050044
        // Special levels
        switch(levelId) {
            case 0x7a:
                // FUN_020173c0(0xd,1);
                // Enemy Check, aka Museum
                return "ene_check_";
            case 0x7b:
                return "koopa3";
            case 0x7c:
                return "koopa2";
            case 0x7d:
                return "kuppa";
            case 0x7e:
                // Cutscene?
                return "lastback";
            case 0x7f:
                return "0x7f unknown multi";
                // 020500a0
                // uVar1 = FUN_02017424();
                // if (0x37 < uVar1) {
                //     uVar1 = 0;
                // }
                // return *(char **)(PTR_DAT_02050108 + uVar1 * 4);
            default:
                break;
        }
    }

    // 020500b8
    if (levelId > 99) {
        return ">99 unknown multi";
    } else {
        const Address LEVEL_FILENAMES_SUB100_ARRAY = YUtils::conv2xAddrToFileAddr(0x020d8e58);
        const uint32_t offset = (levelId - 1) * sizeof(uint32_t);
        Address textPtrAddress = LEVEL_FILENAMES_SUB100_ARRAY + offset;
        Address textAddress = this->getAddrFromAddrPtr(textPtrAddress);
        auto text = this->getTextNullTermAt(textAddress);
        return text;
    }
}

Address YidsRom::getAddrFromAddrPtr(Address pointerAddress_file) {
    this->romFile.seekg(pointerAddress_file);
    uint32_t addr0x2;
    this->romFile.read(reinterpret_cast<char *>(&addr0x2),sizeof(addr0x2));
    return YUtils::conv2xAddrToFileAddr(addr0x2);
}

std::vector<uint8_t> YidsRom::getByteVectorFromFile(std::string fileName) {
    std::vector<uint8_t> vec;
    std::string UNPACKED_FILE_LOCATION = "_nds_unpack/data/file/";
    fileName = UNPACKED_FILE_LOCATION.append(fileName);
    std::ifstream inputFile{fileName, ios::binary};
    std::copy(
        std::istreambuf_iterator<char>(inputFile),
        std::istreambuf_iterator<char>(),
        std::back_inserter(vec)
    );
    return vec;
}

void YidsRom::wipeCrsbData() {
    this->canvasWidthCol = 0;
    this->canvasHeightCol = 0;

    this->pixelTilesBg1.clear();

    this->preRenderDataBg1.clear();
    this->preRenderDataBg2.clear();

    // 1: Skip the universal palette at index 0
    for (uint32_t palDelIndex = 1; palDelIndex < 0x20; palDelIndex++) {
        // 0x10 * 2: Each color is 2 bytes
        for (uint32_t colDelIndex = 0; colDelIndex < (0x10*2); colDelIndex++) {
            // palDelIndex + 1: create a "rainbow" to debug target
            this->currentPalettes[palDelIndex][colDelIndex] = palDelIndex + 1;
        }
    }
}

QByteArray YidsRom::get256Palettes(uint32_t offset) {
    QByteArray qbResult;
    qbResult.resize(256);
    int qbIndex = 0;
    for (int whichPaletteIndex = offset; whichPaletteIndex < 0x20; whichPaletteIndex++) {
        for (int whichColorIndex = 0; whichColorIndex < 0x20; whichColorIndex++) {
            qbResult[qbIndex] = this->currentPalettes[whichPaletteIndex].at(whichColorIndex);
            qbIndex++;
        }
    }
    return qbResult;
}

void YidsRom::moveObject(uint32_t objectUuid, int xOffset, int yOffset) {
    auto levelObjects = this->mapData->getAllLevelObjects();
    for (uint32_t currentObjectIndex = 0; currentObjectIndex < levelObjects.size(); currentObjectIndex++) {
        auto curObject = levelObjects.at(currentObjectIndex);
        if (curObject->uuid == objectUuid) {
            curObject->xPosition += xOffset;
            curObject->yPosition += yOffset;
            return;
        }
    }
    YUtils::printDebug("Could not move object, UUID not found",DebugType::WARNING);
}

void YidsRom::moveObjectTo(uint32_t objectUuid, uint32_t newX, uint32_t newY) {
    auto levelObjects = this->mapData->getAllLevelObjects();
    for (uint32_t currentObjectIndex = 0; currentObjectIndex < levelObjects.size(); currentObjectIndex++) {
        auto curObject = levelObjects.at(currentObjectIndex);
        if (curObject->uuid == objectUuid) {
            curObject->xPosition = newX;
            curObject->yPosition = newY;
            return;
        }
    }
    YUtils::printDebug("Could not move-to object, UUID not found",DebugType::WARNING);
}