#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"
#include "../Chartile.h"

#include <vector>
#include <fstream>
#include <QByteArray>

std::map<uint32_t, Chartile> LayerData::getVramChartiles() {
    if (this->cachedVramTiles.size() > 0) {
        return this->cachedVramTiles;
    }
    std::map<uint32_t, Chartile> pixelTiles;
    uint32_t chartileIndex = 0;
    // Keep it simple
    for (uint32_t i = 0; i < this->subScenData.size(); i++) {
        auto curSection = this->subScenData.at(i);
        if (curSection->getMagic() == Constants::INFO_MAGIC_NUM) {
            auto info = static_cast<ScenInfoData*>(curSection);
            auto imbzFilename = info->imbzFilename; // Similar to IMGB
            if (imbzFilename.empty()) {
                YUtils::printDebug("No IMBZ filename in info, skipping", DebugType::VERBOSE);
                continue;
            }
            auto tileVector = this->parseImbzFromFile(imbzFilename,info->colorMode);
            for (uint j = 0; j < tileVector.size(); j++) {
                pixelTiles[chartileIndex++] = tileVector.at(j);
            }
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
        }
    }
    this->cachedVramTiles = pixelTiles;
    return pixelTiles;
}

ScenInfoData *LayerData::getInfo()
{
    auto potentialInfo = this->getFirstDataByMagic(Constants::INFO_MAGIC_NUM);
    if (potentialInfo == nullptr) {
        YUtils::printDebug("Failed to find SCEN INFO",DebugType::ERROR);
        return nullptr;
    }
    return static_cast<ScenInfoData*>(potentialInfo);
}

LayerPaletteData *LayerData::getPalette() {
    auto potentialPalette = this->getFirstDataByMagic(Constants::PLTB_MAGIC_NUM);
    if (potentialPalette == nullptr) {
        YUtils::printDebug("Failed to find PLTB info",DebugType::ERROR);
        return nullptr;
    }
    return static_cast<LayerPaletteData*>(potentialPalette);
}

LevelData* LayerData::getFirstDataByMagic(uint32_t magicNumber, bool silentFail) {
    for (auto it = this->subScenData.begin(); it != this->subScenData.end(); it++) {
        if ( (*it)->getMagic() == magicNumber ) {
            return (*it);
        }
    }
    if (!silentFail) {
        std::stringstream ss;
        ss << "Sub-LayerData with magic number ";
        ss << std::hex << magicNumber << " not found";
        YUtils::printDebug(ss.str(),DebugType::WARNING);
    }
    return nullptr;
}

std::vector<uint16_t> LayerData::getPreRenderData() {
    auto mpbzMaybe = this->getFirstDataByMagic(Constants::MPBZ_MAGIC_NUM);
    if (mpbzMaybe == nullptr) {
        YUtils::printDebug("MPBZ empty, returning empty vector",DebugType::ERROR);
        return std::vector<uint16_t>();
    }
    auto mpbz = static_cast<MapTilesData*>(mpbzMaybe);
    return mpbz->tileRenderData;
}

std::vector<Chartile> LayerData::parseImbzFromFile(std::string filename_noExt, BgColorMode bgColMode) {
    if (this->cachedImbzTileData.size() > 0) {
        return this->cachedImbzTileData;
    }
    std::vector<Chartile> result;
    // Get the file vector
    std::vector<uint8_t> vec;
    std::string tempFullFileName = filename_noExt.append(".imbz");
    std::string UNPACKED_FILE_LOCATION = "_nds_unpack/data/file/";
    tempFullFileName = UNPACKED_FILE_LOCATION.append(tempFullFileName);
    std::ifstream inputFile{tempFullFileName, ios::binary};
    std::copy(
        std::istreambuf_iterator<char>(inputFile),
        std::istreambuf_iterator<char>(),
        std::back_inserter(vec)
    );
    // Decompress that vector
    std::vector uncompressedImbz = YCompression::lzssVectorDecomp(vec,false);
    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles
    int imbzIndex = 0; // Goes up by 0x20/32 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    if (imbzLength < 1) {
        YUtils::printDebug("imbzLength is 0!",DebugType::ERROR);
        return std::vector<Chartile>();
    }

        // Do it 0x20 by 0x20 (32)
    while (imbzIndex < imbzLength) { // Kill when equal to length, meaning it's outside
        Chartile curTile;
        curTile.index = currentTileIndex;
        curTile.tiles.resize(64);
        if (bgColMode == BgColorMode::MODE_16) {
            curTile.bgColMode = BgColorMode::MODE_16;
            // Go up by 2 since you split the bytes
            for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
                uint8_t curByte = uncompressedImbz.at(imbzIndex + currentTileBuildIndex);
                uint8_t highBit = curByte >> 4;
                uint8_t lowBit = curByte % 0x10;
                int innerPosition = currentTileBuildIndex*2;
                curTile.tiles[innerPosition+1] = highBit;
                curTile.tiles[innerPosition+0] = lowBit;
            }
        } else {
            curTile.bgColMode = BgColorMode::MODE_256;
            for (int currentTileIndex = 0; currentTileIndex < 64; currentTileIndex++) {
                uint8_t curByte = uncompressedImbz.at(imbzIndex + currentTileIndex);
                curTile.tiles[currentTileIndex] = curByte;
            }
        }
        result.push_back(curTile);
        
        if (bgColMode == BgColorMode::MODE_16) {
            // Skip ahead by 0x20/32
            imbzIndex += Constants::CHARTILE_DATA_SIZE;
        } else {
            imbzIndex += 64;
        }
        currentTileIndex++;
    }
    this->cachedImbzFilenameNoExt = filename_noExt;
    this->cachedImbzTileData = result;
    return result;
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

LayerData* MapData::getScenByBg(uint8_t bg) {
    for (auto it = this->subData.begin(); it != this->subData.end(); it++) {
        if ( (*it)->getMagic() == Constants::SCEN_MAGIC_NUM ) {
            LayerData* ld = static_cast<LayerData*>(*it);
            auto info = ld->getInfo();
            if (info->whichBackground == bg) {
                return ld;
            }
        }
    }
    std::stringstream ss;
    ss << "Failed to get SCEN with BG ";
    ss << std::hex << (uint16_t)bg;
    YUtils::printDebug(ss.str(),DebugType::ERROR);
    return nullptr;
}

std::vector<LevelObject *> MapData::getAllLevelObjects() {
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

uint32_t MapData::getGreatestCanvasHeight() {
    uint32_t greatest = 0;
    for (int i = 1; i <= 3; i++) {
        auto possibleScen = this->getScenByBg(i);
        if (possibleScen != nullptr) {
            auto info = possibleScen->getInfo();
            if (info != nullptr) {
                if (info->layerHeight > greatest) {
                    greatest = info->layerHeight;
                }
            }
        }
    }
    return greatest;
}

uint32_t MapData::getGreatestCanvasWidth() {
    uint32_t greatest = 0;
    for (int i = 1; i <= 3; i++) {
        auto possibleScen = this->getScenByBg(i);
        if (possibleScen != nullptr) {
            auto info = possibleScen->getInfo();
            if (info != nullptr) {
                if (info->layerWidth > greatest) {
                    greatest = info->layerWidth;
                }
            }
        }
    }
    return greatest;
}

std::vector<uint8_t> MapData::getCollisionArray() {
    for (int i = 1; i <= 3; i++) {
        auto possibleScen = this->getScenByBg(i);
        if (possibleScen != nullptr) {
            auto possibleColz = possibleScen->getFirstDataByMagic(Constants::COLZ_MAGIC_NUM,true);
            if (possibleColz != nullptr) {
                return static_cast<MapCollisionData*>(possibleColz)->colData;
            }
        }
    }
    YUtils::printDebug("COLZ data not found!",DebugType::ERROR);
    return std::vector<uint8_t>();
}

uint32_t MapData::getCollisionCanvasWidth() {
    for (int i = 1; i <= 3; i++) {
        auto possibleScen = this->getScenByBg(i);
        if (possibleScen != nullptr) {
            auto possibleColz = possibleScen->getFirstDataByMagic(Constants::COLZ_MAGIC_NUM,true);
            if (possibleColz != nullptr) {
                return possibleScen->getInfo()->layerWidth;
            }
        }
    }
    YUtils::printDebug("COLZ data not found!",DebugType::ERROR);
    return 0;
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
