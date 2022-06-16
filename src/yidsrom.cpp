#include "yidsrom.h"
#include "utils.h"
#include "compression.h"
#include "Chartile.h"

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

using namespace std;

// Value pertaining to file-relative address (Starts at 0x0)
typedef uint32_t Address;
// Value pertaining to executable memory (0x02xxxxxx)
typedef uint32_t AddressMemory;

// NitroSDK, crt0.c:193
const uint32_t SDK_NITROCODE_BE	= 0xdec00621;
const uint32_t ARM9_ROM_OFFSET = 0x4000;
const uint32_t MAIN_MEM_OFFSET = 0x0200'0000;

// Universal Palette 0: 0x020d6f40
const AddressMemory UNIVERSAL_PALETTE_0_ADDR = 0x020d6f40;

const char* NEW_BIN_FILE = "bin9.bin";
const char* NEW_ROM_FILE = "rom_uncomp.nds";
const std::string CRSB_MAGIC = "CRSB";
const char* CRSB_EXTENSION = ".crsb";
const std::string CSCN_MAGIC = "CSCN";
const char* MPDZ_EXTENSION = ".mpdz";

const uint32_t MPDZ_MAGIC_NUM = 0x00544553; // "SET "
const uint32_t SCEN_MAGIC_NUM = 0x4e454353; // "SCEN"
const uint32_t INFO_MAGIC_NUM = 0x4f464e49; // "INFO"
const uint32_t PLTB_MAGIC_NUM = 0x42544c50; // "PLTB"
const uint32_t MPBZ_MAGIC_NUM = 0x5a42504d; // "MPBZ" / 4D 50 42 5A

const int PALETTE_SIZE = 0x20;

YidsRom::YidsRom(bool verbose) {
    cout << "YidsRom constructed" << endl;
    this->filesLoaded = false;
    this->verbose = verbose;
    this->pixelTiles.reserve(1000); // Found 988 in 1-1's first IMBZ
}

void YidsRom::openRom(std::string fileName) {
    this->filesLoaded = false;
    this->romFile.open(fileName, ios::binary | ios::in);
    if (!romFile) {
        cerr << "ERROR: ROM file could not be opened: '" << fileName << "'" << endl;
        cerr << "Reported load error code: " << errno << endl;
        exit(EXIT_FAILURE);
    } else {
        if (this->verbose) {
            cout << "File loaded: " << fileName << " [SUCCESS]" << endl;
        }
    }

    /************************
     *** METADATA/HEADERS *** 
     ************************/
    if (this->verbose) cout << "Checking ROM game code... ";
    std::string romCode = this->getTextAt(0x0c,4);
    if (this->verbose) cout << romCode << " ";
    if (romCode.compare(YidsRom::GAME_CODE) != 0) {
        cout << "[FAIL]";
        cerr << "Game code AYWE not found! Got '" << romCode << "' instead. Wrong ROM?" << endl;
        exit(EXIT_FAILURE);
    } else {
        if (this->verbose) cout << "[SUCCESS]" << endl;
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
        cerr << "[FAIL] First subdirectory '" << SUBDIR_NAME << "' was not found! Found '" << subdirName << "' instead.";
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
            cout << hex << "[WARN] Long length: " << (int)len << " at " << hex << (FILESDIR_BASE_ADDR + fileOffset) << endl;
        }
        auto nameStr = this->getTextAt(FILESDIR_BASE_ADDR + fileOffset + 1,len);
        // cout << "len: " << hex << (int)len << ", id: " << dec << curFileId << ", name: '" << nameStr << "'" << endl;
        // When finding files, it lowercases
        this->fileIdMap[YUtils::getLowercase(nameStr)] = curFileId;
        fileOffset += len + 1; // +1 accounts for length byte
        curFileId++;
    }

    std::string crsbFileName = this->getLevelFileNameFromMapIndex(0,0);
    this->loadCrsb(crsbFileName);
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
    uint8_t MAX_STRING_LENGTH = 0xff;
    uint8_t killOffset = 0;
    std::string result = "";

    this->romFile.seekg(position_file + offset);
    char container;
    const uint8_t NULL_TERM = 0x00;
    this->romFile.read(&container, sizeof(container));
    if (container == NULL_TERM) {
        cout << "[WARN] Found empty string at position " << hex << position_file << endl;
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

template<typename T>
T YidsRom::getNumberAt(uint32_t addr) {
    T container;
    this->romFile.seekg(addr);
    this->romFile.read(reinterpret_cast<char *>(&container), sizeof(container));
    return container;
}

YidsRom::~YidsRom() {
    // delete/delete[] things here
}

void YidsRom::initArm9RomData(std::string fileName) {
    // Start of ARM9 ROM data, aka "MainRomOffset"
    this->romFile.seekg(0x20);
    Address romStart9;
    this->romFile.read(reinterpret_cast<char *>(&romStart9),sizeof(romStart9));

    if (this->verbose) cout << "Checking ARM9 ROM offset... 0x" << hex << romStart9 << " ";
    if (ARM9_ROM_OFFSET != romStart9) {
        cout << "[FAIL]" << endl;
        cerr << "Found ARM9 ROM offset not equal to " << hex << ARM9_ROM_OFFSET << endl;
        exit(EXIT_FAILURE);
    } else {
        if (this->verbose) cout << "[SUCCESS]" << endl;
    }

    // Size of ARM9 ROM data, aka "MainSize"
    this->romFile.seekg(0x2c);
    uint32_t romSize9;
    this->romFile.read(reinterpret_cast<char *>(&romSize9),sizeof(romSize9));
    if (this->verbose) cout << "ARM9 ROM Size: 0x" << hex << romSize9 << endl;

    // End of ARM9 ROM (exclusive)
    Address endAddress = romStart9 + romSize9;
    if (this->verbose) cout << "ARM9 ROM End: 0x" << hex << endAddress << endl;

    if (this->verbose) cout << "Checking SDK_NITROCODE_BE (" << SDK_NITROCODE_BE << ")... ";
    this->romFile.seekg(endAddress);
    uint32_t endAddressMagicNumber;
    this->romFile.read(reinterpret_cast<char *>(&endAddressMagicNumber),sizeof(endAddressMagicNumber));
    if (endAddressMagicNumber != SDK_NITROCODE_BE) {
        cout << "[FAIL]" << endl;
        cerr << "SDK_NITROCODE_BE magic number " << SDK_NITROCODE_BE << " not found" << endl;
        exit(EXIT_FAILURE);
    } else {
        if (this->verbose) cout << "[SUCCESS]" << endl;
    } 

    this->writeUncompressedARM9(romStart9,romSize9);
    bool arm9decompResult = YCompression::blzDecompress(NEW_BIN_FILE, this->verbose);
    if (!arm9decompResult) {
        cerr << "Could not decompress the ARM9 BIN!" << endl;
        exit(EXIT_FAILURE);
    }
    std::ifstream arm9fileUncomped;
    arm9fileUncomped.open(NEW_BIN_FILE, ios::binary);

    // Get arm9fileUncomped's length
    arm9fileUncomped.seekg(0, ios_base::end);
    size_t arm9fileLength = arm9fileUncomped.tellg();
    arm9fileUncomped.seekg(0, ios_base::beg);

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
        fileName, NEW_ROM_FILE,
        std::filesystem::copy_options::overwrite_existing
    );

    // Change romFile to new one
    this->romFile.close();
    this->romFile.open(NEW_ROM_FILE, ios::binary | ios::in | ios::out);
    this->romFile.seekp(ARM9_ROM_OFFSET + 0x4000);
    this->romFile.write(reinterpret_cast<char *>(arm9buffer.data()),arm9fileLength);
    arm9fileUncomped.close();

    // Test it
    if (this->verbose) cout << "Testing Decompression and text retrieval... ";
    const Address TEST_POSITION = 0xe9e6e;
    const auto TEST_TEXT = "1-1_D3";
    auto testText1 = this->getTextAt(TEST_POSITION,6);
    if (testText1.compare(TEST_TEXT) != 0) {
        cerr << "[FAIL]" << endl << "Looked for '" << TEST_TEXT << "' with getTextAt, found '" << testText1 << "'" << endl;
        cerr << "File decompression may have failed, or been modified improperly" << endl;
        exit(EXIT_FAILURE);
    }
    auto testText2 = this->getTextNullTermAt(TEST_POSITION);
    if (testText2.compare(TEST_TEXT) != 0) {
        cerr << "[FAIL]" << endl << "Looked for '" << TEST_TEXT << "' with getTextNullTermAt, found '" << testText2 << "'" << endl;
        cerr << "File decompression may have failed, or been modified improperly" << endl;
        exit(EXIT_FAILURE);
    }
    if (this->verbose) cout << "[SUCCESS]" << endl;

    // Delete the old file (Consider streaming to memory instead)
    filesystem::remove(NEW_ROM_FILE);

    // It worked! Report them as loaded successfully
    this->filesLoaded = true;

    // Write universal palette
    this->currentPalettes[0].resize(PALETTE_SIZE);
    Address universalPalette0base = this->conv2xAddrToFileAddr(UNIVERSAL_PALETTE_0_ADDR);
    for (int univPalIndex = 0; univPalIndex < PALETTE_SIZE; univPalIndex++) {
        this->romFile.seekg(universalPalette0base + univPalIndex);
        uint8_t container;
        this->romFile.read(reinterpret_cast<char *>(&container), sizeof(container));
        this->currentPalettes[0][univPalIndex] = container;
    }
}

void YidsRom::writeUncompressedARM9(Address arm9start_rom, uint32_t arm9length) {
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

    YUtils::writeByteVectorToFile(outvec,NEW_BIN_FILE);

    if (this->verbose) cout << "[SUCCESS] Wrote file" << endl;
}

/**
 * @brief Takes in the addresses seen in Ghidra or
 * No$GBA and outputs uncomped ROM-relative address
 * 
 * @param x2address 0x02xxxxxx
 * @return File-relative address
 */
Address YidsRom::conv2xAddrToFileAddr(AddressMemory x2address) {
    return x2address - MAIN_MEM_OFFSET + ARM9_ROM_OFFSET + 0x4000;
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
        const Address LEVEL_FILENAMES_SUB100_ARRAY = this->conv2xAddrToFileAddr(0x020d8e58);
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
    return this->conv2xAddrToFileAddr(addr0x2);
}

void YidsRom::loadCrsb(std::string fileName_noext) {
    fileName_noext = YUtils::getLowercase(fileName_noext);
    auto fileName = fileName_noext.append(".crsb");
    auto fileId = this->fileIdMap[fileName];
    if (fileId == 0) {
        cerr << "Failed to load level: " << fileName << endl;
        return;
    }
    // x8 because each record of ranges is 8 bytes. 4 + 4
    Address rangesAddr = this->metadata.fatOffsets + fileId * 8;
    Address startAddr = this->getNumberAt<uint32_t>(rangesAddr + 0);
    Address endAddr = this->getNumberAt<uint32_t>(rangesAddr + 4);
    uint32_t length = endAddr - startAddr;
    // 0x08 is from device capacity. Understand this better
    uint32_t MAX_SIZE = 1U << (17 + 0x08);
    if (length > MAX_SIZE) {
        cerr << "File is too big! File size: " << dec << length << ", max size: " << dec << MAX_SIZE << endl;
        return;
    }
    
    std::string magicText = this->getTextAt(startAddr + 0,4);
    if (magicText.compare(CRSB_MAGIC) != 0) {
        cerr << "Magic header text " << CRSB_MAGIC << " not found! Found '" << magicText << "' instead." << endl;
        return;
    }
    // Important is MPDZ, which are marked by SCEN, and is the actual data for
    //   each level and sublevel inside a world level. 1-1 has 4, one for each
    //   area, and no more

    // Number of CSCN records, aka maps in level!
    uint16_t mapFileCount = this->getNumberAt<uint16_t>(startAddr + 8);
    if (this->verbose) cout << "Map count: " << mapFileCount << endl;

    const uint32_t OFFSET_TO_FIRST_CSCN = 0xC; // 12

    uint32_t curCscnReadOffset = 0; // In bytes
    for (uint32_t cscnIndex = 0; cscnIndex < mapFileCount; cscnIndex++) {
        // Points to the current magic number text, CSCN
        Address baseAddrCscn = startAddr + OFFSET_TO_FIRST_CSCN + curCscnReadOffset;
        // Check that the magic text is there, at index 0
        std::string magicTextCscn = this->getTextAt(baseAddrCscn + 0, 4);
        if (magicTextCscn.compare(CSCN_MAGIC) != 0) {
            cerr << "Magic header text " << CSCN_MAGIC << " not found! Found '" << magicTextCscn << "' instead." << endl;
            return;
        }
        // Next, get the filename
        auto mpdzFilename_noext = this->getTextNullTermAt(baseAddrCscn + 0xC);
        this->loadMpdz(mpdzFilename_noext);

        // +0x4 is because the magic number is 4 long, and the next is the length
        // +0x8 is because that length is added to the current read position in the file
        //   That recreates it as a non-relative offset number (it started from the
        //   maybeExits at +0x8)
        uint32_t cscnLength = this->getNumberAt<uint32_t>(baseAddrCscn + 0x4) + 0x8;
        
        curCscnReadOffset += cscnLength;
        return;
    }

    if (this->verbose) cout << "All MPDZ files loaded" << endl;
}

void YidsRom::loadMpdz(std::string fileName_noext) {
    if (this->verbose) cout << "Loading MPDZ '" << fileName_noext << "'" << endl;
    std::string mpdzFileName = fileName_noext.append(MPDZ_EXTENSION);
    auto fileVector = this->getFileByteVector(mpdzFileName);
    YUtils::writeByteVectorToFile(fileVector,mpdzFileName);
    bool decompResult = YCompression::lzssDecomp(mpdzFileName, this->verbose);
    if (!decompResult) {
        cerr << "Failed to extract MPDZ file" << endl;
        return;
    }
    fileVector.clear();

    std::ifstream uncomped{mpdzFileName, ios::binary};
    if (!uncomped) {
        cerr << "Failed to load uncompressed file '" << mpdzFileName << "'" << endl;
        return;
    }

    size_t uncomped_length = YUtils::getStreamLength(uncomped);
    std::vector<uint8_t> uncompVec;
    uncompVec.reserve(uncomped_length);
    std::copy(
        std::istreambuf_iterator<char>(uncomped),
        std::istreambuf_iterator<char>(),
        std::back_inserter(uncompVec)
    );

    uint32_t magic = YUtils::getUint32FromVec(uncompVec,0);
    if (magic != MPDZ_MAGIC_NUM) {
        cerr << "MPDZ Magic number not found! Expected " << hex << MPDZ_MAGIC_NUM;
        cerr << ", got " << hex << magic << " instead." << endl;
        return;
    } else {
        if (this->verbose) cout << "[SUCCESS] MPDZ Magic number found" << endl;
    }
    // 4 because the file length is written at bytes 4-7
    uint32_t mpdzFileLength = YUtils::getUint32FromVec(uncompVec, 4);
    // 8 in order to start it at the first instruction besides SET
    Address mpdzIndex = 8; // Pass this in as a pointer to functions
    
    // Instruction loop
    while (mpdzIndex < mpdzFileLength) {
        uint32_t curInstruction = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
        if (curInstruction == SCEN_MAGIC_NUM) {
            this->handleSCEN(uncompVec,mpdzIndex);
        } else {
            cerr << "[WARN] Instruction besides SCEN used: " << hex << curInstruction << endl;
        }
        return; // Temp
    }
}

/**
 * @brief Handles the SCEN instruction from MPDZ files
 * 
 * @param mpdzVec Reference to vector with MPDZ data, all of it
 * @param indexPointer Reference to Address, pointing at the current SCEN instruction
 */
void YidsRom::handleSCEN(std::vector<uint8_t>& mpdzVec, Address& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != SCEN_MAGIC_NUM) {
        cerr << "SCEN instruction did not find magic hex " << hex << SCEN_MAGIC_NUM << endl;
        return;
    }
    indexPointer += sizeof(uint32_t);
    uint32_t scenLength = YUtils::getUint32FromVec(mpdzVec, indexPointer);
    indexPointer += sizeof(uint32_t);
    uint32_t lengthIndex = 0;
    while (lengthIndex < scenLength) {
        uint32_t curSubInstruction = YUtils::getUint32FromVec(mpdzVec,indexPointer);
        if (curSubInstruction == INFO_MAGIC_NUM) {
            uint32_t infoLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First time: 0x20
            
            // TODO: What do these values do?
            uint32_t unknownValue0 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 8); // 00b60208
            uint32_t unknownValue1 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 12); // 00000000
            uint32_t unknownValue2 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 16); // 0x1000
            uint32_t unknownValue3 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 20); // 0x1000
            uint32_t unknownValue4 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 24); // 0x020202
            uint32_t unknownValue5 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 28); // 00000000
            Q_UNUSED(unknownValue0);
            Q_UNUSED(unknownValue1);
            Q_UNUSED(unknownValue2);
            Q_UNUSED(unknownValue3);
            Q_UNUSED(unknownValue4);
            Q_UNUSED(unknownValue5);
            // Get charfile string
            auto charFileNoExt = YUtils::getNullTermTextFromVec(mpdzVec, indexPointer + 32);
            this->handleImbz(charFileNoExt);
            // Increment based on earlier length, +8 is to skip instruction and length
            indexPointer += infoLength + 8;
        } else if (curSubInstruction == PLTB_MAGIC_NUM) {
            uint32_t pltbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First time: 0x1c0
            Address pltbReadIndex = indexPointer + 8; // +8 is to skip instruction and length
            indexPointer += pltbLength + 8; // Do this ahead of time in order to control the while loop
            // Cycle up to the index pointer
            int globalPaletteIndex = 1; // Start at 1 because universal
            while (pltbReadIndex < indexPointer) {
                QByteArray currentLoadingPalette;
                currentLoadingPalette.resize(PALETTE_SIZE);
                for (int curPaletteIndex = 0; curPaletteIndex < PALETTE_SIZE; curPaletteIndex++) {
                    currentLoadingPalette[curPaletteIndex] = mpdzVec.at(pltbReadIndex + curPaletteIndex);
                    // cout << "Loc: " << hex << (pltbReadIndex + curPaletteIndex) << ", val: ";
                    // cout << hex << (int)mpdzVec.at(pltbReadIndex + curPaletteIndex) << ";" << endl;
                }
                // Should round down because of int
                this->currentPalettes[globalPaletteIndex] = currentLoadingPalette;
                globalPaletteIndex++;
                pltbReadIndex += PALETTE_SIZE; // 1 palette is 32 bytes, or 0x20
            }
            // for (int i = 0; i < PALETTE_SIZE; i+=2) {
            //     YUtils::getColorFromBytes(this->currentPalettes[1].at(i),this->currentPalettes[1].at(i+1));
            // }
        } else if (curSubInstruction == MPBZ_MAGIC_NUM) {
            uint32_t mpbzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            // Slice out MPBZ data
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + mpbzLength;
            auto compressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            // Decompress MPBZ data
            auto uncompressedMpbz = YCompression::lzssVectorDecomp(compressedSubArray);
            indexPointer += mpbzLength + 8; // Skip ahead main pointer to next
            // Handle uncompressedMpbz data
            const uint32_t uncompressedMpbzTwoByteCount = uncompressedMpbz.size() / 2;
            //for (int mpbzIndex = 0; mpbzIndex < uncompressedMpbzTwoByteCount; mpbzIndex++) {
            this->preRenderData.reserve(0xff);
            for (int mpbzIndex = 0; mpbzIndex < uncompressedMpbzTwoByteCount; mpbzIndex++) {
                uint32_t trueOffset = mpbzIndex*2;
                uint16_t firstByte = (uint16_t)uncompressedMpbz.at(trueOffset);
                uint16_t secondByte = (uint16_t)uncompressedMpbz.at(trueOffset+1);
                uint16_t curShort = (secondByte << 8) + firstByte;
                this->preRenderData.push_back(curShort);
            }
        } else {
            cout << "Unknown instruction: " << hex << curSubInstruction << endl;
            return;
        }
    }
}
const int CHARTILE_DATA_SIZE = 0x20;
void YidsRom::handleImbz(std::string fileName_noext) {
    if (this->verbose) cout << "Handling IMBZ file: '" << fileName_noext << "'" << endl;
    auto fileVector = this->getFileByteVector(fileName_noext.append(".imbz"));
    std::vector uncompressedImbz = YCompression::lzssVectorDecomp(fileVector);
    fileVector.clear();

    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles. Start at -1 due to first time ++
    int imbzIndex = 0; // Goes up by 0x20 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    // Do it 0x20 by 0x20
    while (imbzIndex < imbzLength) { // Kill when equal to length, meaning it's outside
        Chartile curTile;
        curTile.engine = ScreenEngine::A;
        curTile.index = currentTileIndex;
        curTile.tiles.resize(64);
        // Go up by 2 since you split the bytes
        for (int currentTileBuildIndex = 0; currentTileBuildIndex < CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
            uint8_t curByte = uncompressedImbz.at(imbzIndex + currentTileBuildIndex);
            uint8_t highBit = curByte >> 4;
            uint8_t lowBit = curByte % 0x10;
            int innerPosition = currentTileBuildIndex*2;
            curTile.tiles[innerPosition+1] = highBit;
            curTile.tiles[innerPosition+0] = lowBit;
        }
        this->pixelTiles.push_back(curTile);
        // Skip ahead by 0x20
        imbzIndex += CHARTILE_DATA_SIZE;
        currentTileIndex++;
    }
    // cout << "Total tiles: " << dec << currentTileIndex << endl;
    // for (int i = 0; i < 0x10; i++) {
    //     cout << endl;
    //     for (int j = 0; j < 64; j++) {
    //         cout << setw(2) << hex << (int)this->pixelTiles.at(i).tiles[j] << " ";
    //         if (j % 8 == 0) {
    //             cout << endl;
    //         }
    //     }
    // }
}

std::vector<uint8_t> YidsRom::getFileByteVector(std::string fileName) {
    fileName = YUtils::getLowercase(fileName); // All comparisons are done in lowercase
    auto fileId = this->fileIdMap[fileName];
    if (fileId == 0) {
        cerr << "[FAIL] Could not find fileId for filename '" << fileName << "'" << endl;
        exit(EXIT_FAILURE);
    }
    // x8 because each record of ranges is 8 bytes. 4 + 4
    Address rangesAddr = this->metadata.fatOffsets + fileId * 8;
    Address startAddr = this->getNumberAt<uint32_t>(rangesAddr + 0);
    Address endAddr = this->getNumberAt<uint32_t>(rangesAddr + 4);
    uint32_t length = endAddr - startAddr;
    // 0x08 is from device capacity. Understand this better
    uint32_t MAX_SIZE = 1U << (17 + 0x08);
    if (length > MAX_SIZE) {
        cerr << "File is too big! File size: " << dec << length << ", max size: " << dec << MAX_SIZE << endl;
        exit(EXIT_FAILURE);
    }
    std::vector<uint8_t> outVec(length,0xfe);
    for(uint32_t vecIndex = 0; vecIndex < length; vecIndex++) {
        outVec.at(vecIndex) = this->getNumberAt<uint8_t>(startAddr + vecIndex);
    }
    return outVec;
}