#include "yidsrom.h"
#include "compression.h"
#include "Chartile.h"
#include "utils.h"
#include "constants.h"
#include "LevelObject.h"
#include "PixelDelegate.h"
#include "PixelDelegateEnums.h"
#include "popups/PaletteTable.h"
#include "Level.h"

#include "data/MapData.h"

#include <iostream>
#include <vector>
#include <QFile>

void YidsRom::loadMpdz(std::string fileName_noext) {
    std::stringstream ssMpdzLoad;
    ssMpdzLoad << "Loading MPDZ (Map) '" << fileName_noext << "'";
    YUtils::printDebug(ssMpdzLoad.str(),DebugType::VERBOSE);
    std::string mpdzFileName = fileName_noext.append(Constants::MPDZ_EXTENSION);
    auto fileVector = this->getByteVectorFromFile(mpdzFileName);
    this->mapData = new MapData(fileVector,true,this->backgroundPalettes);
    this->mapData->filename = mpdzFileName;
}

void YidsRom::updateSpriteMeta() {
    YUtils::printDebug("Updating sprite metadata");
    QFile spriteFile(":/sprites.csv");
    if (!spriteFile.open(QIODevice::ReadOnly)) {
        std::stringstream ssMetaFileLoadFail;
        ssMetaFileLoadFail << "Failed to load sprites.csv: " << spriteFile.errorString().toStdString();
        YUtils::printDebug(ssMetaFileLoadFail.str());
        YUtils::popupAlert(ssMetaFileLoadFail.str());
        exit(EXIT_FAILURE);
    }
    this->spriteMetadata = std::vector<SpriteMeta>();
    spriteFile.readLine(); // Skip the header
    while (!spriteFile.atEnd()) {
        QByteArray line = spriteFile.readLine();
        auto stringSplit = line.split(',');
        if (stringSplit.size() < 4) {
            YUtils::printDebug("StringSplit was small, skipping",DebugType::WARNING);
            continue;
        }
        auto spriteId = stringSplit.at(0).toUInt(nullptr,16);
        if (spriteId > 0xffff) {
            YUtils::printDebug("Sprite ID too high, skipping",DebugType::ERROR);
            continue;
        }
        auto spriteName = stringSplit.at(1).toStdString();
        auto textDetails = stringSplit.at(2).toStdString();
        auto settingsLength = stringSplit.at(3).toUInt(nullptr,10);
        if (settingsLength % 4 != 0) {
            YUtils::printDebug("Settings length retrieved was not divisible by 4, skipping",DebugType::ERROR);
            continue;
        }
        SpriteMeta smetta;
        smetta.spriteId = (uint16_t)spriteId;
        smetta.name = spriteName;
        smetta.info = textDetails;
        smetta.createdSettingsLen = settingsLength; // remember, ONLY for new objects
        spriteMetadata.push_back(smetta);
    }
    // spriteMetadata should be filled
}

SpriteMeta YidsRom::getSpriteMetadata(uint32_t spriteId) {
    for (auto it = this->spriteMetadata.begin(); it != this->spriteMetadata.end(); it++) {
        if (it->spriteId == spriteId) {
            return *it;
        }
    }
    //YUtils::printDebug("Sprite metadata not found",DebugType::VERBOSE);
    SpriteMeta blank;
    std::stringstream ssName;
    ssName << "Sprite 0x" << std::hex << spriteId;
    blank.name = ssName.str();
    blank.info = "Undocumented";
    blank.createdSettingsLen = 0xff; // ONLY use this for sprite creation
    blank.spriteId = (uint16_t)spriteId;
    return blank;
}

ObjectFile YidsRom::getObjPltFile(std::string objset_filename) {
    using namespace std;
    ObjectFile objFileData;
    objFileData.objectFileName = objset_filename;
    
    std::stringstream ssGraphics;
    ssGraphics << "Loading graphics archive '" << objset_filename << "'";
    YUtils::printDebug(ssGraphics.str(),DebugType::VERBOSE);
    std::vector<uint8_t> fileVectorObjset = this->getByteVectorFromFile(objset_filename);
    std::vector<uint8_t> objsetUncompressedVec = fileVectorObjset;
    if (fileVectorObjset.at(0) != 0x10) {
        //YUtils::printDebug("Archive not compressed, skipping decomp",DebugType::VERBOSE);
    } else {
        objsetUncompressedVec = YCompression::lz10decomp(fileVectorObjset);
    }

    auto potentialMagicNumber = YUtils::getUint32FromVec(objsetUncompressedVec,0);
    if (potentialMagicNumber != Constants::OBAR_MAGIC_NUM) {
        cerr << "[ERROR] OBAR magic number not found in file '" << objset_filename << "'! Found instead: " << hex << potentialMagicNumber << endl;
        exit(EXIT_FAILURE);
    }
    auto fullObjsetLength = YUtils::getUint32FromVec(objsetUncompressedVec,4);

    uint32_t indexObjset = 8; // magic number (4) + length uint32 (4)

    const uint32_t objsetEndIndex = fullObjsetLength + 8; // Exclusive, but shouldn't matter

    // Accessed universally, not by type index
    uint32_t obarSectionIndex = 0;
    while (indexObjset < objsetEndIndex) {
        auto instructionCheck = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);

        indexObjset += 4; // Skip instruction, go to length
        auto currentInstructionLength = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);
        indexObjset += 4; // Skip length, go to first

        auto subsection = YUtils::subVector(objsetUncompressedVec,indexObjset,indexObjset + currentInstructionLength);
        if (instructionCheck == Constants::OBJB_MAGIC_NUM || instructionCheck == Constants::OBJZ_MAGIC_NUM) {
            /**************
             *** OBJB/Z ***
             **************/
            if (instructionCheck == Constants::OBJZ_MAGIC_NUM) {
                subsection = YCompression::lz10decomp(subsection);
            }
            objFileData.objectPixelTiles[obarSectionIndex] = subsection;
        } else if (instructionCheck == Constants::PLTB_MAGIC_NUM) {
            /************
             *** PLTB ***
             ************/
            uint32_t subSectionSize = subsection.size();
            if (subSectionSize != Constants::PALETTE_SIZE) {
                std::stringstream ssPltbSize;
                ssPltbSize << "PLTB data not 0x20/32 bytes! Was instead: 0x" << std::hex << subSectionSize;
                ssPltbSize << ". Only pulling first one.";
                YUtils::printDebug(ssPltbSize.str(),DebugType::WARNING);
            }
            ObjectPalette currentLoadingPalette;
            currentLoadingPalette.paletteData.resize(Constants::PALETTE_SIZE);
            for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
                currentLoadingPalette.paletteData[curPaletteIndex] = subsection.at(curPaletteIndex);
            }
            currentLoadingPalette.index = obarSectionIndex;
            currentLoadingPalette.address = indexObjset;
            objFileData.objectPalettes[obarSectionIndex] = currentLoadingPalette;
        } else {
            std::cerr << "[ERROR] Known objset magic number not found! Instead found ";
            std::cerr << hex << instructionCheck << " at " << std::hex << (indexObjset - 4) << std::endl;
            exit(EXIT_FAILURE);
        }
        obarSectionIndex++;
        indexObjset += currentInstructionLength;
    }
    if (objFileData.objectPixelTiles.size() < 1) {
        YUtils::printDebug("Pulled zero OBJB records",DebugType::ERROR);
    }
    if (objFileData.objectPalettes.size() < 1) {
        YUtils::printDebug("Pulled zero PLTB records",DebugType::ERROR);
    }
    return objFileData;
}

bool YidsRom::loadObjectRenderFile(std::string obarFileFull) {
    if (obarFileFull.empty()) {
        YUtils::printDebug("Given empty OBAR file name",DebugType::WARNING);
        return false;
    }
    // It's already loaded if the count isn't 0
    if (this->objectRenderFiles.count(obarFileFull) > 0) {
        return false;
    }
    this->objectRenderFiles[obarFileFull] = this->getObjPltFile(obarFileFull);
    return true;
}

void YidsRom::getHintMessageData(uint16_t hintMessageId) {
    YUtils::printDebug("getHintMessageData");
    std::vector<uint8_t> mespack = this->getByteVectorFromFile("mespack.mes");
    // 020ccdc8
    uint index = 0;
    uint dest = 0;
    uint messageTarget = 0;

    uint32_t maxCount = YUtils::getUint32FromVec(mespack,dest);
    dest += 4;
    do {
        uint checkLoc = index * 4; // This is working correctly
        auto checkValue = YUtils::getUint16FromVec(mespack,dest + index*4);
        if (checkValue == hintMessageId || checkValue == 0xffff) {
            break;
        } 
        index++;
        messageTarget = messageTarget + YUtils::getUint16FromVec(mespack,dest + checkLoc + 2);
    } while (index < maxCount);
    uint messageLocation = 0x388 + messageTarget;

    auto headerVector = YUtils::subVector(mespack,messageLocation,messageLocation+0xc);
    YUtils::printVector(headerVector);
    messageLocation += 0xc;

    auto length = YUtils::getUint32FromVec(mespack,messageLocation);
    messageLocation += 4;

    auto compressedVector = YUtils::subVector(mespack,messageLocation,messageLocation+length);
    auto uncomped = YCompression::lz10decomp(compressedVector);
    std::cout << "Uncomped size: 0x" << std::hex << uncomped.size() << std::endl;
    // 0x80 pixels wide, 4bpp single palette
    //YUtils::printVector(uncomped);
}
