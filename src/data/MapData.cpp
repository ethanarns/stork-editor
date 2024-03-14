#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"
#include "../Chartile.h"
#include "../GlobalSettings.h"

#include <vector>
#include <fstream>
#include <QByteArray>

ScenInfoData *LayerData::getInfo() {
    auto potentialInfo = this->getFirstDataByMagic(Constants::INFO_MAGIC_NUM);
    if (potentialInfo == nullptr) {
        YUtils::printDebug("Failed to find SCEN INFO",DebugType::ERROR);
        return nullptr;
    }
    return static_cast<ScenInfoData*>(potentialInfo);
}

LayerPaletteData *LayerData::getPalette() {
    auto potentialPalette = this->getFirstDataByMagic(Constants::PLTB_MAGIC_NUM,true);
    if (potentialPalette == nullptr) {
        // Fail silently, as some SCENs do not have PLTBs and thus just don't contribute
        LayerPaletteData* blankPal = new LayerPaletteData(); // Return empty
        return blankPal;
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

std::vector<MapTileRecordData> LayerData::getMapTiles() {
    auto mpbzMaybe = this->getFirstDataByMagic(Constants::MPBZ_MAGIC_NUM);
    if (mpbzMaybe == nullptr) {
        YUtils::printDebug("MPBZ empty, returning empty vector",DebugType::ERROR);
        return std::vector<MapTileRecordData>();
    }
    auto mpbz = static_cast<MapTilesData*>(mpbzMaybe);
    return mpbz->mapTiles;
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
    std::ifstream inputFile{tempFullFileName, std::ios::binary};
    std::copy(
        std::istreambuf_iterator<char>(inputFile),
        std::istreambuf_iterator<char>(),
        std::back_inserter(vec)
    );
    // Decompress that vector
    std::vector uncompressedImbz = YCompression::lz10decomp(vec);
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

MapData::MapData(std::vector<uint8_t> mpdzBytes, bool compressed, QByteArray bgPalettesRef[0x20]) {
    this->paletteRamIndex = 1;
    globalSettings.temp_paletteOffset = 0;
    if (compressed) {
        mpdzBytes = YCompression::lz10decomp(mpdzBytes);
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
    auto idealMpdzStop = mpdzIndex + mpdzLength;
    while (mpdzIndex < mpdzLength) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        auto idealStop = mpdzIndex+subLength;
        if (subMagic == Constants::SCEN_MAGIC_NUM) {
            auto scen = new LayerData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            scen->getPalette()->bgOffset = globalSettings.temp_paletteOffset; // Set before increment
            auto pltb = scen->getPalette()->palettes;
            for (auto plit = pltb.begin(); plit != pltb.end(); plit++) {
                auto curPal = (*plit);
                bgPalettesRef[this->paletteRamIndex] = curPal;
                this->paletteRamIndex++;
                globalSettings.temp_paletteOffset++;
            }
            this->subData.push_back(scen);
        } else if (subMagic == Constants::GRAD_MAGIC_NUM) {
            auto grad = new LevelGradientData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(grad);
        } else if (subMagic == Constants::SETD_MAGIC_NUM) {
            auto setd = new LevelObjectData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(setd);
        } else if (subMagic == Constants::AREA_MAGIC_NUM) {
            auto area = new TriggerBoxData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(area);
        } else if (subMagic == Constants::PATH_MAGIC_NUM) {
            auto path = new PathData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(path);
        } else if (subMagic == Constants::ALPH_MAGIC_NUM) {
            auto alph = new AlphaData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subData.push_back(alph);
        } else {
            std::stringstream ssSubNotFound;
            ssSubNotFound << "Unknown MPDZ data: ";
            ssSubNotFound << std::hex << YUtils::magicToAscii(subMagic);
            YUtils::printDebug(ssSubNotFound.str(),DebugType::WARNING);
            ssSubNotFound << ", saving the level may corrupt it!";
            YUtils::popupAlert(ssSubNotFound.str());
            mpdzIndex += subLength;
        }
        if (mpdzIndex != idealStop) {
            YUtils::printDebug("MPDZ index did not stop at correct index for magic 0x",DebugType::ERROR);
        }
    }
    if (mpdzIndex != idealMpdzStop) {
        YUtils::printDebug("MPDZ index did not stop at correct place",DebugType::ERROR);
    }
}

MapData::~MapData() {
    for (auto it = this->subData.begin(); it != this->subData.end(); ) {
        delete (*it);
        it = this->subData.erase(it);
    }
    this->subData.shrink_to_fit();
}

LayerData* MapData::getScenByBg(uint8_t bg, bool silentFail) {
    for (auto it = this->subData.begin(); it != this->subData.end(); it++) {
        if ( (*it)->getMagic() == Constants::SCEN_MAGIC_NUM ) {
            LayerData* ld = static_cast<LayerData*>(*it);
            auto info = ld->getInfo();
            if (info->whichBackground == bg) {
                return ld;
            }
        }
    }
    if (!silentFail) {
        std::stringstream ss;
        ss << "Failed to get SCEN with BG ";
        ss << std::hex << (uint16_t)bg;
        YUtils::printDebug(ss.str(),DebugType::WARNING);
    }
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

MapCollisionData* MapData::getCollisionData() {
    for (int i = 1; i <= 3; i++) {
        auto possibleScen = this->getScenByBg(i);
        if (possibleScen != nullptr) {
            auto possibleColz = possibleScen->getFirstDataByMagic(Constants::COLZ_MAGIC_NUM,true);
            if (possibleColz != nullptr) {
                return static_cast<MapCollisionData*>(possibleColz);
            }
        }
    }
    YUtils::printDebug("COLZ data not found!",DebugType::ERROR);
    return nullptr;
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

std::vector<QByteArray> MapData::getBackgroundPalettes(QByteArray universalPalette) {
    std::vector<QByteArray> result;
    result.push_back(universalPalette);
    for (auto subit = this->subData.begin(); subit != this->subData.end(); subit++) {
        if ((*subit)->getMagic() == Constants::SCEN_MAGIC_NUM) {
            auto scen = static_cast<LayerData*>(*subit);
            auto curPalettes = scen->getPalette()->palettes;
            for (uint i = 0; i < curPalettes.size(); i++) {
                result.push_back(curPalettes.at(i));
            }
        }
    }
    return result;
}

QByteArray MapData::getLayerOrder() {
    if (this->layerOrderCache.size() > 0) {
        return this->layerOrderCache;
    }
    QByteArray result;
    result.fill(0,3); // Set result to 3 zeroes
    for (auto it = this->subData.begin(); it != this->subData.end(); it++) {
        if ((*it)->getMagic() == Constants::SCEN_MAGIC_NUM) {
            auto scen = static_cast<LayerData*>((*it));
            auto orderValueForLayer = scen->getInfo()->layerOrder;
            // whichBackground value and layerOrder value OFTEN match, but not always
            // Also, reverse them (4 minus 1 through 3 reverses it), as lower numbers drawn first
            // Then subtract 1 because indexes start at 0 not 1
            // result[4-orderValueForLayer-1] = scen->getInfo()->whichBackground;
            // TODO: What is the layer order value even for??
            // If you get an error about it look here
            Q_UNUSED(orderValueForLayer);
            result[4-scen->getInfo()->whichBackground-1] = scen->getInfo()->whichBackground;
        }
    }
    this->layerOrderCache = result;
    return result;
}

bool MapData::wipeLayerOrderCache() {
    if (this->layerOrderCache.size() == 0) {
        return false;
    } else if (this->layerOrderCache.size() > 3) {
        std::stringstream ss;
        ss << "Layer order cache size greater than 3! Was 0x";
        ss << std::hex << this->layerOrderCache.size();
        YUtils::printDebug(ss.str(),DebugType::ERROR);
    }
    this->layerOrderCache.clear();
    return true;
}

LevelData *MapData::getFirstDataByMagic(uint32_t magicNumber, bool silentFail) {
    for (auto it = this->subData.begin(); it != this->subData.end(); it++) {
        if ( (*it)->getMagic() == magicNumber ) {
            return (*it);
        }
    }
    if (!silentFail) {
        std::stringstream ss;
        ss << "LevelData with magic number ";
        ss << std::hex << magicNumber << " not found";
        YUtils::printDebug(ss.str(),DebugType::WARNING);
    }
    return nullptr;
}

bool MapData::deleteSpriteByUUID(uint32_t uuid) {
    YUtils::printDebug("deleteSpriteByUUID");
    auto setdMaybe = this->getFirstDataByMagic(Constants::SETD_MAGIC_NUM);
    if (setdMaybe == nullptr) {
        YUtils::printDebug("SETD not found in deleteSpriteByUUID",DebugType::ERROR);
        YUtils::popupAlert("SETD not found when deleting sprite");
        return false;
    }
    auto setd = static_cast<LevelObjectData*>(setdMaybe);
    for (auto it = setd->levelObjects.begin(); it != setd->levelObjects.end(); it++) {
        auto sprite = (*it);
        if (sprite->uuid == uuid) {
            YUtils::printDebug("UUID found, freeing memory and removing");
            delete (*it);
            setd->levelObjects.erase(it);
            return true;
        }
    }
    YUtils::printDebug("Sprite not found to delete",DebugType::ERROR);
    return false;
}

LevelObject* MapData::addSpriteData(LevelObject lo) {
    YUtils::printDebug("Adding new sprite data to MapData",DebugType::VERBOSE);
    auto newLevelObject = new LevelObject(lo);
    auto setdMaybe = this->getFirstDataByMagic(Constants::SETD_MAGIC_NUM);
    if (setdMaybe == nullptr) {
        YUtils::printDebug("SETD not found in addSpriteData",DebugType::ERROR);
        YUtils::popupAlert("SETD not found when adding sprite");
        return newLevelObject;
    }
    auto setd = static_cast<LevelObjectData*>(setdMaybe);
    // Ensure unique UUID
    newLevelObject->uuid = setd->uuidIndex++;
    setd->levelObjects.push_back(newLevelObject);
    return newLevelObject;
}
