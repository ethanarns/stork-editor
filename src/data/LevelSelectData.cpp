#include "LevelSelectData.h"
#include "../utils.h"

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

LevelSelectData::LevelSelectData(std::vector<uint8_t> crsbBytes) {
    std::cout << "LevelSelectData" << std::endl;
    uint32_t crsbIndex = 0;
    auto magic = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
    crsbIndex += 4;
    if (magic != Constants::CRSB_MAGIC_NUM) {
        std::stringstream ssBadMagic1;
        ssBadMagic1 << "Bad magic number when reading CRSB file: ";
        ssBadMagic1 << std::hex << magic;
        YUtils::printDebug(ssBadMagic1.str(),DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    auto crsbLength = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
    Q_UNUSED(crsbLength);
    crsbIndex += 4;
    auto numberOfCscnFiles = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
    Q_UNUSED(numberOfCscnFiles);
    crsbIndex += 4;
    auto crsbEnd = crsbBytes.size();
    while (crsbIndex < crsbEnd) {
        auto levelMeta = new LevelMetadata();
        auto cscnMagic = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
        crsbIndex += 4;
        if (cscnMagic != Constants::CSCN_MAGIC_NUM) {
            std::stringstream ssBadMagic2;
            ssBadMagic2 << "Bad magic number when reading CSCN file: ";
            ssBadMagic2 << std::hex << cscnMagic;
            YUtils::printDebug(ssBadMagic2.str(),DebugType::FATAL);
            exit(EXIT_FAILURE);
        }
        auto cscnLength = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
        Q_UNUSED(cscnLength);
        crsbIndex += 4;
        auto numMapEnters = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
        crsbIndex += 2;
        auto exitPortalCount = crsbBytes.at(crsbIndex);
        crsbIndex++;
        levelMeta->musicId = (LevelSelectEnums::MapMusicId)crsbBytes.at(crsbIndex);
        crsbIndex++;
        // This is at offset 0xC
        auto mpdzFilename = YUtils::getNullTermTextFromVec(crsbBytes,crsbIndex);
        levelMeta->mpdzFileNoExtension = mpdzFilename;
        // Hard boost by 0x10 (16)
        crsbIndex += 0x10; // 02033224
        uint enterIntoMapIndex = 0;
        while (enterIntoMapIndex < numMapEnters) {
            auto curEnter = new MapEnterIntoMap(); // We want to be able to edit it
            curEnter->entranceX = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            curEnter->entranceY = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            auto returnAnimAndScreen = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            curEnter->screen = returnAnimAndScreen >> 14;
            curEnter->enterMapAnimation = (LevelSelectEnums::MapExitAnimation)(returnAnimAndScreen % 0x1000);
            levelMeta->entrances.push_back(curEnter);
            enterIntoMapIndex++;
        }
        while (crsbIndex % 4 != 0) {
            crsbIndex++;
        }
        std::cout << "Current offset inside CRSB: " << std::hex << crsbIndex << std::endl;
        uint exitIndex = 0;
        // This is exactly 8 bytes, no need to pad
        while (exitIndex < exitPortalCount) {
            auto curExit = new MapExitData();
            curExit->exitLocationX = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            curExit->exitLocationY = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            curExit->exitStartType = (LevelSelectEnums::MapExitStartType)YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            curExit->whichMapTo = crsbBytes.at(crsbIndex);
            crsbIndex++;
            curExit->whichEntranceTo = crsbBytes.at(crsbIndex);
            crsbIndex++;
            levelMeta->exits.push_back(curExit);
            exitIndex++; // Loop increment
        }
    }
}