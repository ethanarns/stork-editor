#include "LevelSelectData.h"
#include "../utils.h"

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

// CRSB, go over this again at some point
LevelSelectData::LevelSelectData(std::vector<uint8_t> crsbBytes) {
    //std::cout << "LevelSelectData" << std::endl;
    if (crsbBytes.size() == 0) {
        YUtils::printDebug("CRSB size was 0",DebugType::FATAL);
        YUtils::popupAlert("LevelSelectData size was 0");
        exit(EXIT_FAILURE);
    }
    uint32_t crsbIndex = 0;
    auto magic = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
    crsbIndex += 4;
    if (magic != Constants::CRSB_MAGIC_NUM) {
        std::stringstream ssBadMagic1;
        ssBadMagic1 << "Bad magic number when reading CRSB file: ";
        ssBadMagic1 << std::hex << magic;
        YUtils::printDebug(ssBadMagic1.str(),DebugType::FATAL);
        YUtils::popupAlert(ssBadMagic1.str());
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
            YUtils::popupAlert(ssBadMagic2.str());
            exit(EXIT_FAILURE);
        }
        auto cscnLength = YUtils::getUint32FromVec(crsbBytes,crsbIndex);
        Q_UNUSED(cscnLength);
        crsbIndex += 4;
        auto numMapEnters = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
        crsbIndex += 2;
        auto exitPortalCount = crsbBytes.at(crsbIndex);
        crsbIndex++;
        levelMeta->musicId = crsbBytes.at(crsbIndex);
        crsbIndex++;
        // This is at offset 0xC
        auto mpdzFilename = YUtils::getNullTermTextFromVec(crsbBytes,crsbIndex);
        levelMeta->mpdzFileNoExtension = mpdzFilename;
        // Hard boost by 0x10 (16)
        crsbIndex += 0x10; // 02033224
        uint enterIntoMapIndex = 0;
        while (enterIntoMapIndex < numMapEnters) {
            auto curEnter = new MapEntrance(); // We want to be able to edit it
            curEnter->entranceX = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            curEnter->entranceY = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            auto returnAnimAndScreen = YUtils::getUint16FromVec(crsbBytes,crsbIndex);
            crsbIndex += 2;
            uint16_t whichScreenUint = (returnAnimAndScreen >> 14);
            curEnter->whichDsScreen = static_cast<LevelSelectEnums::StartingDsScreen>(whichScreenUint);
            curEnter->enterMapAnimation = (LevelSelectEnums::MapEntranceAnimation)(returnAnimAndScreen % 0x1000);
            curEnter->_uuid = this->_portalUuid++;
            levelMeta->entrances.push_back(curEnter);
            enterIntoMapIndex++;
        }
        while (crsbIndex % 4 != 0) {
            crsbIndex++;
        }
        //std::cout << "Current offset inside CRSB: " << std::hex << crsbIndex << std::endl;
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
            curExit->_uuid = this->_portalUuid++;
            levelMeta->exits.push_back(curExit);
            exitIndex++; // Loop increment
        }
        this->levels.push_back(levelMeta);
    }
    for (auto lit = this->levels.begin(); lit != this->levels.end(); lit++) {
        auto curLevel = (*lit);
        for (auto crexit = curLevel->exits.begin(); crexit != curLevel->exits.end(); crexit++) {
            auto exitData = (*crexit);
            auto mapName = this->levels.at(exitData->whichMapTo)->mpdzFileNoExtension;
            exitData->_whichMapToName = mapName;
            auto entrancesOfThatMap = this->levels.at(exitData->whichMapTo)->entrances;
            auto entranceOfThatMap = entrancesOfThatMap.at(exitData->whichEntranceTo);
            exitData->_whichEntranceToUuid = entranceOfThatMap->_uuid;
        }
    }
}

LevelMetadata *LevelSelectData::getLevelByMpdz(std::string mpdzFilename) {
    for (auto it = this->levels.begin(); it != this->levels.end(); it++) {
        auto mpdzNoExt = (*it)->mpdzFileNoExtension;
        mpdzNoExt.append(".mpdz");
        if (mpdzFilename.compare(mpdzNoExt) == 0) {
            return *it;
        }
    }
    YUtils::printDebug("No level with that MPDZ found",DebugType::ERROR);
    return nullptr;
}

int LevelSelectData::getIndexOfEntranceInMpdz(uint32_t entranceUuid, std::string mpdzNameNoExt) {
    auto curLevel = this->getLevelByMpdz(mpdzNameNoExt); //this->yidsRom->currentLevelSelectData->getLevelByMpdz(this->yidsRom->mapData->filename);
    if (curLevel == nullptr) {
        YUtils::printDebug("Current level data is null",DebugType::ERROR);
        return -1;
    }
    for (int i = 0; i < (int)curLevel->entrances.size(); i++) {
        if (curLevel->entrances.at(i)->_uuid == entranceUuid) {
            return i;
        }
    }
    YUtils::printDebug("Entrance index not found",DebugType::ERROR);
    return -1;
}

int LevelSelectData::getIndexOfExitInMpdz(uint32_t exitUuid, std::string mpdzNameNoExt) {
    auto curLevel = this->getLevelByMpdz(mpdzNameNoExt);
    if (curLevel == nullptr) {
        YUtils::printDebug("Current level data is null",DebugType::ERROR);
        return -1;
    }
    for (int i = 0; i < (int)curLevel->exits.size(); i++) {
        if (curLevel->exits.at(i)->_uuid == exitUuid) {
            return i;
        }
    }
    YUtils::printDebug("Exit index not found",DebugType::ERROR);
    return -1;
}