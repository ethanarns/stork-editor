#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"
#include "../Chartile.h"

#include <vector>
#include <fstream>
#include <QByteArray>

LayerData::LayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    // YUtils::printDebug("LayerData loop start",DebugType::VERBOSE);
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
            auto imgb = new ImgbLayerData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(imgb);
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

ScenInfoData* LayerData::getInfo() {
    // FIXME
    return nullptr;
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
            auto scen = new LayerData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(scen);
        } else if (subMagic == Constants::GRAD_MAGIC_NUM) {
            auto grad = new LevelGradientData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(grad);
        } else if (subMagic == Constants::SETD_MAGIC_NUM) {
            auto setd = new LevelObjectData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(setd);
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
        //YUtils::printDebug("No IMBZ string",DebugType::VERBOSE);
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

ImgbLayerData::ImgbLayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    uint32_t debugIndex = 0;
    while (mpdzIndex < stop) {
        Chartile curTile;
        curTile.bgColMode = BgColorMode::MODE_16;
        curTile.index = debugIndex;
        debugIndex++;
        curTile.tiles.resize(64);
        for (
            int currentTileBuildIndex = 0;
            currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE;
            currentTileBuildIndex++
        ) {
            uint8_t curByte = mpdzBytes.at(mpdzIndex);
            mpdzIndex++;
            uint8_t highBit = curByte >> 4;
            uint8_t lowBit = curByte % 0x10;
            int innerPosition = currentTileBuildIndex*2;
            curTile.tiles[innerPosition+1] = highBit;
            curTile.tiles[innerPosition+0] = lowBit;
        }
        this->chartiles.push_back(curTile);
    }
}

LevelGradientData::LevelGradientData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    // It is GINF then GCOL every single time
    while (mpdzIndex < stop) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t tempEnd = mpdzIndex + subLength;
        if (subMagic == Constants::GINF_MAGIC_NUM) {
            this->unknown1 = mpdzBytes.at(mpdzIndex++);
            this->unknown2 = mpdzBytes.at(mpdzIndex++);
            this->unknown3 = mpdzBytes.at(mpdzIndex++);
            this->unknown4 = mpdzBytes.at(mpdzIndex++);
            this->unknown5 = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 2;
            this->unknown6 = mpdzBytes.at(mpdzIndex++);
            this->unknown7 = mpdzBytes.at(mpdzIndex++);
            this->yOffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 4;
        } else if (subMagic == Constants::GCOL_MAGIC_NUM) {
            uint32_t stop = mpdzIndex+subLength;
            while (mpdzIndex < stop) {
                auto curColor = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
                mpdzIndex += 2;
                this->colors.push_back(curColor);
            }
        } else {
            std::stringstream unknownMagic;
            unknownMagic << "Unknown magic number in LevelGradientData: 0x" << std::hex << subMagic;
            YUtils::printDebug(unknownMagic.str(),DebugType::ERROR);
            mpdzIndex += subLength;
        }
        if (mpdzIndex != tempEnd) {
            std::stringstream ssEndNotMatch;
            ssEndNotMatch << "Mismatch in end index. Current Index: " << std::hex;
            ssEndNotMatch << mpdzIndex << ", Temp End: " << tempEnd;
            ssEndNotMatch << ", Magic Number in GRAD: " << std::hex << subMagic;
            YUtils::printDebug(ssEndNotMatch.str(),DebugType::ERROR);
        }
    }
}

LevelObjectData::LevelObjectData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    while (mpdzIndex < stop) {
        auto lo = new LevelObject();
        lo->uuid = this->uuidIndex++;
        lo->objectId = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        lo->settingsLength = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        lo->xPosition = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        lo->yPosition = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        for (uint16_t i = 0; i < lo->settingsLength; i++) {
            auto curSetByte = mpdzBytes.at(mpdzIndex);
            mpdzIndex++;
            lo->settings.push_back(curSetByte);
        }
        // Done!
        this->levelObjects.push_back(lo);
    }
    if (mpdzIndex != stop) {
        YUtils::printDebug("Mismatch in SETD end address",DebugType::ERROR);
    }
}

LayerData* MapData::getScenByBg(int bg) {
    for (auto it = this->subData.begin(); it != this->subData.end(); it++) {
        if ( (*it)->getMagic() == Constants::SCEN_MAGIC_NUM ) {
            LayerData* ld = static_cast<LayerData*>(*it);
            // FIXME
        }
    }
    return nullptr;
}

std::vector<LevelObject *> MapData::getAllLevelObjects()
{
    auto potentialSetd = this->getFirstDataByMagic(Constants::SETD_MAGIC_NUM);
    if (potentialSetd == nullptr) {
        YUtils::printDebug("SETD record not found, returning empty",DebugType::ERROR);
        return std::vector<LevelObject *>();
    }
    LevelObjectData* setd = static_cast<LevelObjectData*>(potentialSetd);
    return setd->levelObjects;
}

LevelObject *MapData::getLevelObjectByUuid(uint32_t uuid) {
    auto objects = this->getAllLevelObjects();
    for (auto it = objects.begin(); it != objects.end(); it++) {
        if ( (*it)->uuid == uuid ) {
            return (*it);
        }
    }
    std::stringstream ss;
    ss << "Could not locate LevelObject with UUID 0x";
    ss << std::hex << uuid;
    YUtils::printDebug(ss.str(),DebugType::ERROR);
    return nullptr;
}

LevelData *MapData::getFirstDataByMagic(uint32_t magicNumber) {
    for (auto it = this->subData.begin(); it != this->subData.end(); it++) {
        if ( (*it)->getMagic() == magicNumber ) {
            return (*it);
        }
    }
    std::stringstream ss;
    ss << "LevelData with magic number ";
    ss << std::hex << magicNumber << " not found";
    YUtils::printDebug(ss.str(),DebugType::WARNING);
    return nullptr;
}
