#pragma once

#include "../utils.h"

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <sstream>

namespace LevelSelectEnums {
    enum MapEntranceAnimation {
        JUMP_IN_FROM_LEFT = 0x00,
        JUMP_IN_FROM_RIGHT = 0x09,
        OUT_OF_PIPE_UPWARDS = 0x0B
    };
    // Reference: https://www.youtube.com/watch?v=Zb9jvSQg9sw
    // Usually comes in as uint8_t, but this is stored as int
    // To find these, break on 2013208, load 1-1, and edit 0231e307
    // These potentially point to the ACTUAL music IDs... But with some changes?
    enum MapMusicId {
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

    enum MapExitStartType {
        WALK_TO_LEFT = 0x01,
        PRESS_DOWN_PIPE = 0x04
    };

    enum MapYoshiStartScreen {
        START_TOP_MAYBE = 0,
        START_TOP = 1,
        START_BOTTOM = 2
    };
};

/**
 * This is within CSCN files
*/
struct MapExitData {
    uint16_t exitLocationX;
    uint16_t exitLocationY;
    LevelSelectEnums::MapExitStartType exitStartType;
    uint8_t whichMapTo;
    uint8_t whichEntranceTo;
    std::string toString() {
        std::stringstream ssExit;
        ssExit << "MapExitData { Exit loc x/y: 0x" << std::hex << this->exitLocationX;
        ssExit << "/0x" << std::hex << this->exitLocationY << ", ";
        ssExit << "whichMapTo: 0x" << std::hex << (int)this->whichMapTo << ", exitStartType: 0x" << std::hex << this->exitStartType;
        ssExit << ", whichMapEntranceTo: 0x" << std::hex << (int)this->whichEntranceTo << " }";
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

/**
 * @brief This determines where Yoshi will spawn into levels. 
 * It includes where he first jumps into the stage, but also refers
 * to places where he returns to this level (ie coming back out of
 * a shy guy pipe)
 */
struct MapEntrance {
    uint16_t entranceX;
    uint16_t entranceY;
    LevelSelectEnums::MapEntranceAnimation enterMapAnimation;
    uint8_t whichDsScreen; // CscnYoshiStartScreen
    std::string toString() {
        std::stringstream ssReturnEnter;
        ssReturnEnter << "MapEntrance { x/y spawn: 0x" << std::hex << this->entranceX;
        ssReturnEnter << "/0x" << std::hex << this->entranceY << ", ";
        ssReturnEnter << "Entrance animation: 0x" << std::hex << this->enterMapAnimation << ", ";
        ssReturnEnter << "Starting DS screen: 0x" << std::hex << (uint16_t)this->whichDsScreen << " }";
        return ssReturnEnter.str();
    }
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        auto entranceXvec = YUtils::uint16toVec(this->entranceX);
        auto entranceYvec = YUtils::uint16toVec(this->entranceY);
        YUtils::appendVector(result,entranceXvec);
        YUtils::appendVector(result,entranceYvec);
        uint16_t thirdWord = (uint16_t)this->enterMapAnimation; // The right part is now here
        thirdWord += ((uint16_t)this->whichDsScreen) << 14; // Make the 0x80xx
        auto thirdWordVec = YUtils::uint16toVec(thirdWord);
        YUtils::appendVector(result,thirdWordVec);
        return result;
    }
};

// CSCN
struct LevelMetadata {
    LevelSelectEnums::MapMusicId musicId; // 1 byte, but enums are stored as 4 bytes
    std::string mpdzFileNoExtension;
    std::vector<MapEntrance*> entrances;
    std::vector<MapExitData*> exits;
};

// CRSB
class LevelSelectData {
public:
    LevelSelectData(std::vector<uint8_t> crsbBytes);
    std::vector<LevelMetadata*> levels;
};