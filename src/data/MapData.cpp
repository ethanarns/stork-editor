#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"

#include <vector>
#include <fstream>

LayerData::LayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    while (mpdzIndex < stop) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        if (subMagic == Constants::INFO_MAGIC_NUM) {
            auto info = new ScenInfoData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(info);
        } else {
            mpdzIndex += subLength;
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
    mpdzIndex += 4;
    this->layerHeight = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
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
    mpdzIndex++;
    if (mpdzIndex == stop) {
        YUtils::printDebug("No IMBZ string",DebugType::VERBOSE);
        return;
    }
    auto charFileNoExt = YUtils::getNullTermTextFromVec(mpdzBytes,mpdzIndex);
    this->imbzFilename = charFileNoExt;
    while (mpdzIndex % 4 != 0) {
        mpdzIndex++;
    }
}
