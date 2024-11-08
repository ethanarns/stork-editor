#include "yidsrom.h"
#include "utils.h"
#include "compression.h"
#include "Chartile.h"
#include "constants.h"
#include "data/LevelSelectData.h"
#include "GlobalSettings.h"

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

#include <filesystem>

// Q_UNUSED
#include <QtGlobal>
#include <QByteArray>

YidsRom::YidsRom() {
    this->filesLoaded = false;
}

void YidsRom::openRom(std::string fileName) {
    YUtils::printDebug("Opening ROM");
    auto compRom = YUtils::getUint8VectorFromFile(fileName);
    this->filesLoaded = false;

    /************************
     *** METADATA/HEADERS ***
     ************************/
    std::string romCode = YUtils::getFixedTextFromVec(compRom,0x0c,4);
    if (romCode.compare(YidsRom::GAME_CODE) != 0) {
        std::stringstream ssGameCode;
        ssGameCode << "Game code AYWE not found! Got '" << romCode << "' instead. Wrong ROM?";
        YUtils::printDebug(ssGameCode.str(),DebugType::FATAL);
        YUtils::popupAlert(ssGameCode.str());
        exit(EXIT_FAILURE);
    }

    // http://problemkaputt.de/gbatek.htm#dscartridgeheader
    // NDSTool can check these (first 4 checked, correct)
    this->metadata.fatTableOffset = YUtils::getUint32FromVec(compRom,0x40);
    this->metadata.fatTableSize = YUtils::getUint32FromVec(compRom,0x44);
    this->metadata.fatOffsets = YUtils::getUint32FromVec(compRom,0x48);
    this->metadata.fatSize = YUtils::getUint32FromVec(compRom,0x4c);

    // Decompress the ARM9 ROM
    this->initArm9RomData(fileName, compRom);

    /*************************
     *** FILE MANIPULATION ***
     *** I use files now, but this is still cool so I'm keeping it here anyway hahaha ***
     *************************/

    // // http://problemkaputt.de/gbatek.htm#dscartridgenitroromandnitroarcfilesystems
    // uint32_t offsetToFirstSubTable = YUtils::getUint32FromVec(this->uncompedRomVector,this->metadata.fatTableOffset + 0);
    // Address fileSubDirectory = offsetToFirstSubTable + this->metadata.fatTableOffset;
    // uint8_t subdirTypeAndNameLength = this->uncompedRomVector.at(fileSubDirectory);
    // uint8_t subdirNameLength = subdirTypeAndNameLength % 0x10;
    // // +1 because the text name starts at 1
    // auto subdirName = YUtils::getFixedTextFromVec(this->uncompedRomVector,fileSubDirectory + 1,subdirNameLength);
    // const std::string SUBDIR_NAME = "file";
    // if (subdirName.compare(SUBDIR_NAME) != 0) {
    //     std::stringstream ssFailSub;
    //     ssFailSub << "[FAIL] First subdirectory '" << SUBDIR_NAME << "' was not found! Found '" << subdirName << "' instead.";
    //     YUtils::printDebug(ssFailSub.str(), DebugType::FATAL);
    //     YUtils::popupAlert(ssFailSub.str());
    //     exit(EXIT_FAILURE);
    // }
    // // +4 because 4 bytes between end of text and first entry
    // Address FILESDIR_BASE_ADDR = fileSubDirectory + (uint32_t)subdirNameLength + 4;

    // const uint32_t FIRST_FILE_ID = 45; // 2D

    // const Address FAT_TABLE_END = this->metadata.fatTableOffset + this->metadata.fatTableSize - 1; // Sub 1 to prevent blank end
    // uint32_t fileOffset = 0;
    // uint32_t curFileId = FIRST_FILE_ID;
    // while (FILESDIR_BASE_ADDR + fileOffset < FAT_TABLE_END) {
    //     uint8_t len = this->uncompedRomVector.at(FILESDIR_BASE_ADDR + fileOffset);
    //     if (len > 0x20) {
    //         std::stringstream ssLongLen;
    //         ssLongLen << "Long length: " << std::hex << (int)len << " at " << std::hex << (FILESDIR_BASE_ADDR + fileOffset);
    //         YUtils::printDebug(ssLongLen.str(),DebugType::WARNING);
    //     }
    //     auto nameStr = YUtils::getFixedTextFromVec(this->uncompedRomVector,FILESDIR_BASE_ADDR + fileOffset + 1,len);
    //     // When finding files, it lowercases
    //     this->fileIdMap[YUtils::getLowercase(nameStr)] = curFileId;
    //     fileOffset += len + 1; // +1 accounts for length byte
    //     curFileId++;
    // }

    // Large and common, so preload
    this->loadObjectRenderFile("objset.arcz");

    this->updateSpriteMeta();

    // May replace this with manual
    std::string crsbFileName = this->getLevelFileNameFromMapIndex(0,0);

    // New way
    auto fileNameCrsb_noext = YUtils::getLowercase(crsbFileName);
    auto crsbFilenameExt = fileNameCrsb_noext.append(".crsb");
    auto crsbFileVector = this->getByteVectorFromFile(crsbFilenameExt);
    // YUtils::printVector(crsbFileVector); // compare to below to ensure recompiles right?
    this->currentLevelSelectData = new LevelSelectData(crsbFileVector);
    // auto crsbTestVec = this->currentLevelSelectData->compile();
    // YUtils::printVector(crsbTestVec);
    this->currentLevelSelectData->filename = crsbFilenameExt;
    this->loadMpdz(this->currentLevelSelectData->levels.at(0)->mpdzFileNoExtension);
}

YidsRom::~YidsRom() {
    // delete/delete[] things here
}

void YidsRom::initArm9RomData(std::string fileName, std::vector<uint8_t> &compedRom) {
    YUtils::printDebug("Initializing ARM9 data");
    // Start of ARM9 ROM data, aka "MainRomOffset"
    // this->romFile.seekg(0x20);
    // Address romStart9;
    // this->romFile.read(reinterpret_cast<char *>(&romStart9),sizeof(romStart9));

    // if (Constants::ARM9_ROM_OFFSET != romStart9) {
    //     std::stringstream ssRomOffset;
    //     ssRomOffset << "Found ARM9 ROM offset not equal to ";
    //     ssRomOffset << std::hex << Constants::ARM9_ROM_OFFSET;
    //     YUtils::printDebug(ssRomOffset.str(),DebugType::FATAL);
    //     exit(EXIT_FAILURE);
    // }

    // // Size of ARM9 ROM data, aka "MainSize"
    // this->romFile.seekg(0x2c);
    // uint32_t romSize9;
    // this->romFile.read(reinterpret_cast<char *>(&romSize9),sizeof(romSize9));

    // // End of ARM9 ROM (exclusive)
    // Address endAddress = romStart9 + romSize9;

    // this->romFile.seekg(endAddress);
    // uint32_t endAddressMagicNumber;
    // this->romFile.read(reinterpret_cast<char *>(&endAddressMagicNumber),sizeof(endAddressMagicNumber));
    // if (endAddressMagicNumber != Constants::SDK_NITROCODE_BE) {
    //     std::stringstream ssMn;
    //     ssMn << "SDK_NITROCODE_BG magic number '" << std::hex << Constants::SDK_NITROCODE_BE << "' not found. ";
    //     ssMn << "Instead found " << std::hex << endAddressMagicNumber;
    //     YUtils::printDebug(ssMn.str(),DebugType::FATAL);
    //     exit(EXIT_FAILURE);
    // }
    uint32_t romSize9 = YUtils::getUint32FromVec(compedRom,0x2c);

    // Writes to "bin9.bin"
    // arm9.bin from NDSTool just has 12 extra bytes
    //this->extractCompressedARM9(romStart9,romSize9);
    std::stringstream ssArm9Path;
    ssArm9Path << "./" << globalSettings.extractFolderName << "/arm9.bin";
    auto arm9toolPath = std::filesystem::absolute(ssArm9Path.str());
    auto newBinFilePath = std::filesystem::absolute(Constants::NEW_BIN_FILE);
    if (!std::filesystem::exists(newBinFilePath)) {
        YUtils::printDebug("Decompressing ARM9 Binary");
        std::filesystem::copy(
            arm9toolPath, newBinFilePath,
            std::filesystem::copy_options::overwrite_existing
        );
        std::filesystem::resize_file(newBinFilePath,romSize9);
        bool arm9decompResult = YCompression::blzDecompress(Constants::NEW_BIN_FILE);
        if (!arm9decompResult) {
            YUtils::printDebug("Could not BLZ decompress the ARM9 binary",DebugType::ERROR);
            YUtils::popupAlert("Could not BLZ decompress the ARM9 binary");
            exit(EXIT_FAILURE);
        }
    } else {
        YUtils::printDebug("Decompressed ARM9 Binary already present, skipping decompress");
    }

    std::ifstream arm9fileUncomped;
    arm9fileUncomped.open(Constants::NEW_BIN_FILE, std::ios::binary);

    // Get arm9fileUncomped's length
    arm9fileUncomped.seekg(0, std::ios_base::end);
    size_t arm9fileLength = arm9fileUncomped.tellg();
    arm9fileUncomped.seekg(0, std::ios_base::beg);
    if (arm9fileLength == 0) {
        YUtils::popupAlert("ARM9 binary loaded empty");
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

    // Open uncomped rom file
    std::fstream romFile;
    romFile.open(Constants::NEW_ROM_FILE, std::ios::binary | std::ios::in | std::ios::out);
    romFile.seekp(Constants::ARM9_ROM_OFFSET + 0x4000);
    romFile.write(reinterpret_cast<char *>(arm9buffer.data()),arm9fileLength);
    arm9fileUncomped.close();
    romFile.close();

    this->uncompedRomVector = YUtils::getUint8VectorFromFile(Constants::NEW_ROM_FILE);

    std::filesystem::remove(Constants::NEW_ROM_FILE);
    std::filesystem::remove(newBinFilePath);

    YUtils::printDebug("Testing ARM9 data");

    // Test it
    const Address TEST_POSITION = 0xe9e6e;
    const auto TEST_TEXT = "1-1_D3";
    auto testText1 = YUtils::getFixedTextFromVec(this->uncompedRomVector,TEST_POSITION,6);
    if (testText1.compare(TEST_TEXT) != 0) {
        std::stringstream ssTestText1;
        ssTestText1 << "Test looked for '" << TEST_TEXT << "' with getFixedTextFromVec, found '" << testText1 << "'";
        YUtils::printDebug(ssTestText1.str(),DebugType::FATAL);
        YUtils::popupAlert(ssTestText1.str());
        exit(EXIT_FAILURE);
    }
    auto testText2 = YUtils::getNullTermTextFromVec(this->uncompedRomVector,TEST_POSITION);
    if (testText2.compare(TEST_TEXT) != 0) {
        std::stringstream ssTestText2;
        ssTestText2 << "Test looked for '" << TEST_TEXT << "' with getNullTermTextFromVec, found '" << testText2 << "'";
        YUtils::printDebug(ssTestText2.str(),DebugType::FATAL);
        YUtils::popupAlert(ssTestText2.str());
        exit(EXIT_FAILURE);
    }

    // It worked! Report them as loaded successfully
    this->filesLoaded = true;

    YUtils::printDebug("ARM9 data loaded successfully");

    // Write universal palette
    this->backgroundPalettes[0].resize(Constants::PALETTE_SIZE);
    this->universalPalette.resize(Constants::PALETTE_SIZE);
    Address universalPalette0base = YUtils::conv2xAddrToFileAddr(Constants::UNIVERSAL_PALETTE_0_ADDR);
    for (int univPalIndex = 0; univPalIndex < Constants::PALETTE_SIZE; univPalIndex++) {
        uint32_t seekPos = universalPalette0base + univPalIndex;
        // this->romFile.seekg(universalPalette0base + univPalIndex);
        // uint8_t container;
        // this->romFile.read(reinterpret_cast<char *>(&container), sizeof(container));
        // std::cout << (uint16_t)container << std::endl;
        // std::cout << (uint16_t)this->uncompedRomVector.at(seekPos) << std::endl;
        // exit(EXIT_SUCCESS);
        this->backgroundPalettes[0][univPalIndex] = this->uncompedRomVector.at(seekPos);
        this->universalPalette[univPalIndex] = this->uncompedRomVector.at(seekPos);
    }
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
        std::cerr << "ERROR: World 5 is the highest world level there is" << std::endl;
        return "";
    }
    if (levelIndex + 1 > 10) {
        std::cerr << "ERROR: There are only 10 levels per world" << std::endl;
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
        auto text = YUtils::getNullTermTextFromVec(this->uncompedRomVector,textAddress);
        return text;
    }
}

Address YidsRom::getAddrFromAddrPtr(Address pointerAddress_file) {
    // this->romFile.seekg(pointerAddress_file);
    // uint32_t addr0x2;
    // this->romFile.read(reinterpret_cast<char *>(&addr0x2),sizeof(addr0x2));
    uint32_t addr0x2 = YUtils::getUint32FromVec(this->uncompedRomVector,pointerAddress_file);
    return YUtils::conv2xAddrToFileAddr(addr0x2);
}

std::vector<uint8_t> YidsRom::getByteVectorFromFile(std::string fileName) {
    //std::cout << "getByteVectorFromFile: " << fileName << std::endl;
    std::vector<uint8_t> vec;
    std::stringstream ssUnpackedFileLoc;
    ssUnpackedFileLoc << globalSettings.extractFolderName << "/data/file/";
    std::string UNPACKED_FILE_LOCATION = ssUnpackedFileLoc.str();
    fileName = UNPACKED_FILE_LOCATION.append(fileName);
    std::ifstream inputFile{fileName, std::ios::binary};
    if (!inputFile) {
        std::stringstream ss;
        ss << "getByteVectorFromFile failed: " << fileName;
        YUtils::printDebug(ss.str(),DebugType::FATAL);
        YUtils::popupAlert("Failed to get byte vector from file");
        exit(EXIT_FAILURE);
    }
    std::copy(
        std::istreambuf_iterator<char>(inputFile),
        std::istreambuf_iterator<char>(),
        std::back_inserter(vec)
    );
    return vec;
}

void YidsRom::wipeLevelData() {
    // 1: Skip the universal palette at index 0
    // TODO: Something else?
    // for (uint32_t palDelIndex = 1; palDelIndex < 0x20; palDelIndex++) {
    //     // 0x10 * 2: Each color is 2 bytes
    //     for (uint32_t colDelIndex = 0; colDelIndex < (0x10*2); colDelIndex++) {
    //         // palDelIndex + 1: create a "rainbow" to debug target
    //         //this->currentPalettes[palDelIndex][colDelIndex] = palDelIndex + 1;
    //         this->backgroundPalettes[palDelIndex][colDelIndex] = palDelIndex + 1;
    //     }
    // }
    delete this->mapData;
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
    std::stringstream ssMove;
    ssMove << "Could not move object, UUID not found: 0x" << std::hex << objectUuid;
    YUtils::popupAlert(ssMove.str());
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
    std::stringstream ssMove;
    ssMove << "Could not move-to object, UUID not found: 0x" << std::hex << objectUuid;
    YUtils::popupAlert(ssMove.str());
}

void YidsRom::reloadChartileVram(uint frame) {
    Q_UNUSED(frame);
    if (this->mapData == nullptr) {
        YUtils::printDebug("Chartile VRAM update failed: no MapData",DebugType::ERROR);
        return;
    }
    for (uint8_t whichbg = 1; whichbg <= 3; whichbg++) {
        auto scen = this->mapData->getScenByBg(whichbg);
        if (scen == nullptr) {
            YUtils::printDebug("SCEN for bg not found, skipping",DebugType::WARNING);
            continue;
        }
        auto charBaseBlock = scen->getInfo()->charBaseBlock;
        uint32_t magicOfChartilesSource = 0;
        std::map<uint32_t, Chartile> pixelTiles;
        uint32_t chartileIndex = 0;
        // Loop through the SCEN subdata //
        for (uint32_t i = 0; i < scen->subScenData.size(); i++) {
            auto curSection = scen->subScenData.at(i);
            if (curSection->getMagic() == Constants::INFO_MAGIC_NUM) {
                auto info = static_cast<ScenInfoData*>(curSection);
                auto imbzFilename = info->imbzFilename; // Similar to IMGB
                if (imbzFilename.empty()) {
                    //YUtils::printDebug("No IMBZ filename in info, skipping", DebugType::VERBOSE);
                    continue;
                }
                auto tileVector = scen->parseImbzFromFile(imbzFilename,info->colorMode);
                for (uint j = 0; j < tileVector.size(); j++) {
                    pixelTiles[chartileIndex++] = tileVector.at(j);
                }
                if (magicOfChartilesSource != 0) {
                    YUtils::printDebug("Another SCEN chartile section loaded before INFO-IMBZ",DebugType::ERROR);
                    YUtils::popupAlert("Another SCEN chartile section loaded before INFO-IMBZ");
                }
                magicOfChartilesSource = Constants::INFO_MAGIC_NUM;
            } else if (curSection->getMagic() == Constants::ANMZ_MAGIC_NUM) {
                auto anmz = static_cast<AnimatedMapData*>(curSection);
                chartileIndex = (uint32_t)anmz->vramOffset;
                auto tileVector = anmz->chartiles;
                for (uint j = 0; j < tileVector.size(); j++) {
                    pixelTiles[chartileIndex++] = tileVector.at(j);
                }
                chartileIndex = 0; // Reset after ANMZ
            } else if (curSection->getMagic() == Constants::IMGB_MAGIC_NUM) {
                auto imbz = static_cast<ImgbLayerData*>(curSection);
                auto tileVector = imbz->chartiles;
                for (uint j = 0; j < tileVector.size(); j++) {
                    pixelTiles[chartileIndex++] = tileVector.at(j);
                }
                if (magicOfChartilesSource != 0) {
                    YUtils::printDebug("Another SCEN chartile section loaded before IMGB",DebugType::ERROR);
                    YUtils::popupAlert("Another SCEN chartile section loaded before IMGB");
                }
                magicOfChartilesSource = Constants::IMGB_MAGIC_NUM;
            } else if (curSection->getMagic() == Constants::IMBZ_MAGIC_NUM) {
                auto imbz = static_cast<ImbzLayerData*>(curSection);
                auto tileVector = imbz->chartiles;
                for (uint j = 0; j < tileVector.size(); j++) {
                    pixelTiles[chartileIndex++] = tileVector.at(j);
                }
                if (magicOfChartilesSource != 0) {
                    YUtils::printDebug("Another SCEN chartile section loaded before IMBZ",DebugType::ERROR);
                    YUtils::popupAlert("Another SCEN chartile section loaded before IMBZ");
                }
                magicOfChartilesSource = Constants::IMBZ_MAGIC_NUM;
            }
        }

        this->chartileVram[charBaseBlock] = pixelTiles;
        this->chartileVramPaletteOffset[charBaseBlock] = scen->getPalette()->bgOffset;
    }
    
}
