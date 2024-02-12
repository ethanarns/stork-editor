#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"
#include "../Chartile.h"

#include <vector>
#include <fstream>
#include <QByteArray>

ScenInfoData* LayerData::getInfo() {
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

LevelData* LayerData::getFirstDataByMagic(uint32_t magicNumber) {
    for (auto it = this->subScenData.begin(); it != this->subScenData.end(); it++) {
        if ( (*it)->getMagic() == magicNumber ) {
            return (*it);
        }
    }
    std::stringstream ss;
    ss << "Sub-LayerData with magic number ";
    ss << std::hex << magicNumber << " not found";
    YUtils::printDebug(ss.str(),DebugType::WARNING);
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
