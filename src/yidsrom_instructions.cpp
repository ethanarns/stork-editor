#include "yidsrom.h"
#include "compression.h"
#include "Chartile.h"
#include "utils.h"
#include "constants.h"
#include "LevelObject.h"
#include "PixelDelegate.h"
#include "popups/PaletteTable.h"
#include "Level.h"

#include "data/MapData.h"

#include <iostream>
#include <vector>

void YidsRom::loadMpdz(std::string fileName_noext) {
    using namespace std;
    MapFile mf;
    std::stringstream ssMpdzLoad;
    ssMpdzLoad << "Loading MPDZ (Map) '" << fileName_noext << "'";
    YUtils::printDebug(ssMpdzLoad.str(),DebugType::VERBOSE);
    std::string mpdzFileName = fileName_noext.append(Constants::MPDZ_EXTENSION);
    auto fileVector = this->getByteVectorFromFile(mpdzFileName);
    this->mapData = new MapData(fileVector,true,this->backgroundPalettes);
    this->mapData->filename = mpdzFileName;
}

ObjectFile YidsRom::getMajorObjPltFile(std::string objset_filename, std::map<uint32_t,std::vector<uint8_t>>& pixelTiles, std::map<uint32_t,ObjectPalette>& palettes) {
    ObjectFile objFileData;
    objFileData.objectFileName = objset_filename;
    
    std::stringstream ssGraphics;
    ssGraphics << "Loading graphics archive '" << objset_filename << "'";
    YUtils::printDebug(ssGraphics.str(),DebugType::VERBOSE);
    std::vector<uint8_t> fileVectorObjset = this->getByteVectorFromFile(objset_filename);
    if (fileVectorObjset.size() == 0) {
        YUtils::printDebug("Graphics archive is empty!",DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    std::vector<uint8_t> objsetUncompressedVec = YCompression::lz10decomp(fileVectorObjset);

    auto potentialMagicNumber = YUtils::getUint32FromVec(objsetUncompressedVec,0);
    if (potentialMagicNumber != Constants::OBAR_MAGIC_NUM) {
        std::cerr << "[ERROR] OBAR magic number not found in file '" << objset_filename << "'! Found instead: " << std::hex << potentialMagicNumber << std::endl;
        exit(EXIT_FAILURE);
    }
    auto fullObjsetLength = YUtils::getUint32FromVec(objsetUncompressedVec,4);

    uint32_t indexObjset = 8; // magic number (4) + length uint32 (4)

    const uint32_t objsetEndIndex = fullObjsetLength + 8; // Exclusive, but shouldn't matter

    uint32_t currentPaletteIndex = 0;
    uint32_t curTileStartOffset = 0;
    while (indexObjset < objsetEndIndex) {
        auto instructionCheck = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);

        indexObjset += 4; // Skip instruction, go to length
        auto currentInstructionLength = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);
        indexObjset += 4; // Skip length, go to first

        auto subsection = YUtils::subVector(objsetUncompressedVec,indexObjset,indexObjset + currentInstructionLength);
        if (instructionCheck == Constants::OBJB_MAGIC_NUM) {
            /************
             *** OBJB ***
             ************/
            pixelTiles[curTileStartOffset] = subsection;
            objFileData.objectPixelTiles[curTileStartOffset] = subsection;
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
            currentLoadingPalette.index = currentPaletteIndex;
            currentPaletteIndex++;
            currentLoadingPalette.address = indexObjset;
            // Does not start at zero! Access is offset by 
            palettes[curTileStartOffset] = currentLoadingPalette;
            objFileData.objectPalettes[curTileStartOffset] = currentLoadingPalette;
        } else {
            std::cerr << "[ERROR] Known objset magic number not found! Instead found ";
            std::cerr << std::hex << instructionCheck << " at " << std::hex << (indexObjset - 4) << std::endl;
            exit(EXIT_FAILURE);
        }
        curTileStartOffset++;
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

    uint32_t currentPaletteIndex = 0;
    uint32_t curTileStartOffset = 0;
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
            objFileData.objectPixelTiles[curTileStartOffset] = subsection;
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
            currentLoadingPalette.index = currentPaletteIndex;
            currentLoadingPalette.address = indexObjset;
            objFileData.objectPalettes[currentPaletteIndex] = currentLoadingPalette;
            currentPaletteIndex++;
        } else {
            std::cerr << "[ERROR] Known objset magic number not found! Instead found ";
            std::cerr << hex << instructionCheck << " at " << std::hex << (indexObjset - 4) << std::endl;
            exit(EXIT_FAILURE);
        }
        curTileStartOffset++;
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
