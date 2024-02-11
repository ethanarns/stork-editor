#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"

#include <vector>
#include <fstream>

ScenData::ScenData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex) {
    
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
            ScenData scen = ScenData(mpdzBytes,mpdzIndex);
        } else if (subMagic == Constants::GRAD_MAGIC_NUM) {
            YUtils::printDebug("Handling GRAD",DebugType::VERBOSE);
        } else if (subMagic == Constants::SETD_MAGIC_NUM) {
            YUtils::printDebug("Handling SETD",DebugType::VERBOSE);
        } else {
            std::stringstream ssSubNotFound;
            ssSubNotFound << "Unknown MPDZ data: ";
            ssSubNotFound << std::hex << subMagic;
            YUtils::printDebug(ssSubNotFound.str(),DebugType::WARNING);
        }
    }
}