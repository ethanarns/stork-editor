#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"
#include "../Chartile.h"

#include <vector>
#include <fstream>
#include <QByteArray>

LayerData::LayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    YUtils::printDebug("LayerData loop start",DebugType::VERBOSE);
    while (mpdzIndex < stop) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t tempEnd = mpdzIndex + subLength;
        if (subMagic == Constants::INFO_MAGIC_NUM) {
            auto info = new ScenInfoData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(info);
        } else if (subMagic == Constants::ANMZ_MAGIC_NUM) {
            auto anmz = new AnimatedMapData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(anmz);
        } else if (subMagic == Constants::IMGB_MAGIC_NUM) {
            YUtils::printDebug("IMGB",DebugType::VERBOSE);
            mpdzIndex += subLength;
        } else if (subMagic == Constants::PLTB_MAGIC_NUM) {
            auto pltb = new LayerPaletteData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(pltb);
        } else if (subMagic == Constants::COLZ_MAGIC_NUM) {
            auto colz = new MapCollisionData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(colz);
        } else if (subMagic == Constants::MPBZ_MAGIC_NUM) {
            auto mpbz = new MapTilesData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(mpbz);
        } else if (subMagic == Constants::IMBZ_MAGIC_NUM) {
            YUtils::printDebug("IMBZ",DebugType::VERBOSE);
            mpdzIndex += subLength;
        } else {
            std::stringstream unknownMagic;
            unknownMagic << "Unknown magic number in LayerData: 0x" << std::hex << subMagic;
            YUtils::printDebug(unknownMagic.str(),DebugType::ERROR);
            mpdzIndex += subLength;
        }
        if (mpdzIndex != tempEnd) {
            std::stringstream ssEndNotMatch;
            ssEndNotMatch << "Mismatch in end index. Current Index: " << std::hex;
            ssEndNotMatch << mpdzIndex << ", Temp End: " << tempEnd;
            ssEndNotMatch << ", Magic Number: " << std::hex << subMagic;
            YUtils::printDebug(ssEndNotMatch.str(),DebugType::ERROR);
        }
    }
}

MapData::MapData(std::vector<uint8_t> mpdzBytes, bool compressed) {
    if (compressed) {
        mpdzBytes = YCompression::lzssVectorDecomp(mpdzBytes,false);
    }
    uint32_t mpdzIndex = 0;
    uint32_t magic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    if (magic != Constants::MPDZ_MAGIC_NUM) {
        std::stringstream ssMagic;
        ssMagic << "Wrong magic number. Wanted 0x00544553, got ";
        ssMagic << std::hex << magic;
        YUtils::printDebug(ssMagic.str(),DebugType::ERROR);
        return;
    }
    uint32_t mpdzLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    while (mpdzIndex < mpdzLength) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        if (subMagic == Constants::SCEN_MAGIC_NUM) {
            YUtils::printDebug("Handling SCEN",DebugType::VERBOSE);
            auto scen = new LayerData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(scen);
            YUtils::printDebug(scen->toString(),DebugType::VERBOSE);
        } else if (subMagic == Constants::GRAD_MAGIC_NUM) {
            YUtils::printDebug("Handling GRAD",DebugType::VERBOSE);
            mpdzIndex += subLength;
        } else if (subMagic == Constants::SETD_MAGIC_NUM) {
            YUtils::printDebug("Handling SETD",DebugType::VERBOSE);
            mpdzIndex += subLength;
        } else {
            std::stringstream ssSubNotFound;
            ssSubNotFound << "Unknown MPDZ data: ";
            ssSubNotFound << std::hex << subMagic;
            YUtils::printDebug(ssSubNotFound.str(),DebugType::WARNING);
            mpdzIndex += subLength;
        }
    }
}

ScenInfoData::ScenInfoData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    this->layerWidth = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 2;
    this->layerHeight = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 2;
    this->bgYoffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    this->xScrollOffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    this->yScrollOffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    this->whichBackground = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->layerOrder = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->unkThird = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->baseBlockMaybe = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->colorModeMaybe = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    if (mpdzIndex == stop) {
        YUtils::printDebug("No IMBZ string",DebugType::VERBOSE);
        return;
    }
    auto charFileNoExt = YUtils::getNullTermTextFromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += charFileNoExt.size() + 1;
    this->imbzFilename = charFileNoExt;
    while (mpdzIndex % 4 != 0) {
        mpdzIndex++;
    }
}

LayerPaletteData::LayerPaletteData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    while (mpdzIndex < stop) {
        auto currentLoadingPalette = new QByteArray();
        currentLoadingPalette->resize(Constants::PALETTE_SIZE);
        for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
            (*currentLoadingPalette)[curPaletteIndex] = mpdzBytes.at(mpdzIndex);
            mpdzIndex++;
        }
        this->palettes.push_back(currentLoadingPalette);
    }
}

MapTilesData::MapTilesData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    auto compressed = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    auto mpbzData = YCompression::lzssVectorDecomp(compressed);
    if (YUtils::getUint16FromVec(mpdzBytes,mpdzIndex) == 0xffff) {
        mpdzIndex += 2;
        // Offset is active
        this->tileOffset = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        this->bottomTrim = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
    } else {
        this->tileOffset = 0;
        this->bottomTrim = 0;
    }
    // Loop
    while (mpdzIndex < stop) {
        uint16_t curShort = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        // TODO
        // if (bgColMode == BgColorMode::MODE_16) {
        //     curShort += 0x1000; // 0201c730
        // }
        this->tileRenderData.push_back(curShort);
    }
}

AnimatedMapData::AnimatedMapData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    auto compressed = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    auto anmzData = YCompression::lzssVectorDecomp(compressed);
    const uint32_t anmzSize = anmzData.size();
    uint32_t anmzIndex = 0;
    this->frameCount = anmzData.at(anmzIndex);
    anmzIndex++;
    this->unknown1 = anmzData.at(anmzIndex);
    anmzIndex++;
    this->unknown2 = YUtils::getUint16FromVec(anmzData,anmzIndex);
    anmzIndex += 2;
    this->vramOffset = YUtils::getUint16FromVec(anmzData,anmzIndex);
    anmzIndex += 2;
    this->unknown3 = anmzData.at(anmzIndex);
    anmzIndex++;
    this->unknown4 = anmzData.at(anmzIndex);
    anmzIndex++;
    this->frameTimes.clear();
    for (uint32_t i = 0; i < this->frameCount; i++) {
        this->frameTimes.push_back(anmzData.at(anmzIndex));
        anmzIndex++;
    }
    // 4 byte padding
    while (anmzIndex % 4 != 0) {
        anmzIndex++;
    }
    uint32_t currentTileIndex = 0;
    while (anmzIndex < anmzSize) {
        Chartile curTile;
        curTile.bgColMode = BgColorMode::MODE_16;
        curTile.index = currentTileIndex;
        curTile.tiles.resize(64);
        for (
            int currentTileBuildIndex = 0;
            currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE;
            currentTileBuildIndex++
        ) {
            uint8_t curByte = anmzData.at(anmzIndex);
            anmzIndex++;
            uint8_t highBit = curByte >> 4;
            uint8_t lowBit = curByte % 0x10;
            int innerPosition = currentTileBuildIndex*2;
            curTile.tiles[innerPosition+1] = highBit;
            curTile.tiles[innerPosition+0] = lowBit;
        }
        this->chartiles.push_back(curTile);
    }
    // Index was separate
    mpdzIndex += compressed.size();
}

MapCollisionData::MapCollisionData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    auto compressed = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    this->colData = YCompression::lzssVectorDecomp(compressed);
    mpdzIndex += compressed.size();
}