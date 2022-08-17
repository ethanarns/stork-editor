/**
 * @file Level.h
 * @author your name (you@domain.com)
 * @brief Holds all the data from levels
 * @version 0.1
 * @date 2022-08-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "utils.h"
#include "FsPacker.h"

using namespace std;

enum ExitStartType {
    WALK_TO_LEFT = 0x01,
    PRESS_DOWN_PIPE = 0x04
};

enum CscnYoshiStartScreen {
    START_TOP_MAYBE = 0,
    START_TOP = 1,
    START_BOTTOM = 2
};

enum ExitAnimation {
    JUMP_INTO_LEVEL = 0x00,
    JUMP_OUT_TO_LEFT = 0x09,
    OUT_OF_PIPE_UPWARDS = 0x0B
};

/**
 * @brief This determines where Yoshi will spawn into levels. 
 * It includes where he first jumps into the stage, but also refers
 * to places where he returns to this level (ie coming back out of
 * a shy guy pipe)
 */
struct CscnEnterIntoMap {
    uint16_t entranceX;
    uint16_t entranceY;
    ExitAnimation enterMapAnimation;
    uint8_t screen; // CscnYoshiStartScreen
    std::string toString() {
        std::stringstream ssReturnEnter;
        ssReturnEnter << "CscnEnterIntoMap { x/y loc of spawn: " << std::hex << this->entranceX;
        ssReturnEnter << "/" << std::hex << this->entranceY << ", ";
        ssReturnEnter << "entering map anim: " << std::hex << this->enterMapAnimation << ", ";
        ssReturnEnter << "which screen: " << std::hex << (int)this->screen << " }";
        return ssReturnEnter.str();
    }
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        auto entranceXvec = YUtils::uint16toVec(this->entranceX);
        auto entranceYvec = YUtils::uint16toVec(this->entranceY);
        YUtils::appendVector(result,entranceXvec);
        YUtils::appendVector(result,entranceYvec);
        uint16_t thirdWord = (uint16_t)this->enterMapAnimation; // The right part is now here
        thirdWord += ((uint16_t)this->screen) << 14; // Make the 0x80xx
        auto thirdWordVec = YUtils::uint16toVec(thirdWord);
        YUtils::appendVector(result,thirdWordVec);
        return result;
    }
};

struct CscnExitData {
    uint16_t exitLocationX;
    uint16_t exitLocationY;
    ExitStartType exitStartType;
    uint8_t whichMapTo;
    uint8_t whichEntranceTo;
    std::string toString() {
        std::stringstream ssExit;
        ssExit << "CscnExitData { target x/y: " << std::hex << this->exitLocationX;
        ssExit << "/" << std::hex << this->exitLocationY << ", ";
        ssExit << "whichMapTo: " << std::hex << (int)this->whichMapTo << ", exitStartType: " << std::hex << this->exitStartType;
        ssExit << ", whichMapEntranceTo: " << std::hex << (int)this->whichEntranceTo << " }";
        return ssExit.str();
    }
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        auto exitLocX = YUtils::uint16toVec(this->exitLocationX);
        auto exitLocY = YUtils::uint16toVec(this->exitLocationY);
        auto exitStartTypeVec = YUtils::uint16toVec((uint16_t)this->exitStartType);
        YUtils::appendVector(result,exitLocX);
        YUtils::appendVector(result,exitLocY);
        YUtils::appendVector(result,exitStartTypeVec);
        result.push_back(this->whichMapTo);
        result.push_back(this->whichEntranceTo);
        return result;
    }
};

// Reference: https://www.youtube.com/watch?v=Zb9jvSQg9sw
// Usually comes in as uint8_t, but this is stored as int
// To find these, break on 2013208, load 1-1, and edit 0231e307
// These potentially point to the ACTUAL music IDs... But with some changes?
enum CscnMusicId {
    FLOWER_GARDEN_COPY_1 = 0x00,
    STORY_MUSIC_BOX_WINDUP = 0x01,
    YOSHIS_ISLAND_DS = 0x02,
    FLOWER_FIELD = 0x03,
    YOSHIS_ISLAND_DS_COPY_1 = 0x04,
    YOSHIS_ISLAND_DS_COPY_2 = 0x05,
    TRAINING_COURSE = 0x06,
    SCORE = 0x07,
    MINIGAME = 0x08,
    FLOWER_GARDEN = 0x09,
    UNDERGROUND = 0x0A,
    SEA_COAST = 0x0B,
    JUNGLE = 0x0C,
    CASTLE_AND_FORTRESS = 0x0D,
    IN_THE_CLOUDS = 0x0E,
    WILDLANDS = 0x0F
};

struct CscnData {
    // (jump in, fly in from coin running level near growblock, shyguy pipe 1, shyguy pipe 2)
    uint16_t numMapEnters; // 1-1: 0x0004
    // Pipe to coin running, shy guy pipe 1, shy guy pipe 2
    uint8_t numExitsInScene; // 1-1: 0x03
    CscnMusicId musicId; // 1 byte, but enums are stored as 4 bytes
    std::string mpdzFileNoExtension;
    // 8 zeroes
    std::vector<CscnEnterIntoMap> entrances;
    std::vector<CscnExitData> exits;
    std::string toString() {
        std::stringstream ssCscn;
        ssCscn << "CscnData { exit count: " << hex << this->numMapEnters << ", ";
        ssCscn << "mpdz filename: " << this->mpdzFileNoExtension << " }";
        return ssCscn.str();
    };
    std::vector<uint8_t> compile() {
        // Update these just in case
        this->numExitsInScene = this->exits.size();
        this->numMapEnters = this->entrances.size();
        // Begin compiling
        std::vector<uint8_t> result;
        auto numMapEntersVec = YUtils::uint16toVec(this->numMapEnters);
        result = numMapEntersVec;
        result.push_back(this->numExitsInScene);
        result.push_back((uint8_t)this->musicId);
        auto mapVec = YUtils::stringToVector(this->mpdzFileNoExtension);
        YUtils::appendVector(result,mapVec);
        // 0x14: 02033224
        // 0x04: Skip pre-string bytes
        const uint32_t numZeroes = 0x14 - mapVec.size() - 0x4;
        // numZeroes should pretty much always be 8 (doesn't include null terminator)
        for (uint32_t zeroIndex = 0; zeroIndex < numZeroes; zeroIndex++) {
            result.push_back(0);
        }
        for (uint32_t entrancesIndex = 0; entrancesIndex < this->entrances.size(); entrancesIndex++) {
            auto enterVec = this->entrances.at(entrancesIndex).compile();
            YUtils::appendVector(result,enterVec);
        }
        // See 02033238-02033240 in ARM9 code
        uint32_t enterVecSize = this->entrances.size() * 6;
        uint32_t finalExitsOffset = (enterVecSize + 3) & 0xFFFFFFFC;
        const uint32_t spacerZeroes = (finalExitsOffset - enterVecSize);
        for (uint32_t spacerIndex = 0; spacerIndex < spacerZeroes; spacerIndex++) {
            result.push_back(0);
        }
        for (uint32_t exitsIndex = 0; exitsIndex < this->numExitsInScene; exitsIndex++) {
            auto exitVec = this->exits.at(exitsIndex).compile();
            YUtils::appendVector(result,exitVec);
        }
        result = FsPacker::packInstruction(Constants::CSCN_MAGIC_NUM,result,false);
        return result;
    };
};

struct CrsbData {
    uint32_t mapFileCount;
    std::vector<CscnData> cscnList;
    std::string toString() {
        std::stringstream ssCrsb;
        ssCrsb << "CrsbData { map count value: " << hex << this->mapFileCount << ", ";
        ssCrsb << "loaded maps: " << hex << cscnList.size();
        ssCrsb << " }";
        return ssCrsb.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> resultData;
        const uint32_t sceneCount = this->cscnList.size();
        this->mapFileCount = sceneCount; // Update just in case
        resultData = YUtils::uint32toVec(this->mapFileCount);
        for (uint32_t cscnIndex = 0; cscnIndex < sceneCount; cscnIndex++) {
            auto cscnVec = this->cscnList.at(cscnIndex).compile();
            YUtils::appendVector(resultData,cscnVec);
        }
        auto result = FsPacker::packInstruction(Constants::CRSB_MAGIC_NUM,resultData,false);
        return result;
    };
};

#endif