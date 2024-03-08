#pragma once

#include "../utils.h"

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <sstream>

namespace LevelSelectEnums {    
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
        PRESS_DOWN_PIPE = 0x04,
        BLUE_DOOR = 0x05,
        PRESS_LEFT_PIPE = 0x0B,
        AREA_TRIGGER_UP = 0x0C
    };

    enum MapEntranceAnimation {
        JUMP_IN_FROM_LEFT = 0x00, // Default for starting levels
        SLOW_FALL = 0x04, // Slowly fall for a bit, then gravity resumes. "Big pipes" or ground holes
        FLY_UP_LEFT = 0x09, // Being shot up from underground, or going up to a cloud area usually
        OUT_OF_PIPE_UPWARDS = 0x0B,
        OUT_OF_PIPE_DOWNWARDS = 0x0C
    };

    enum CscnYoshiStartScreen {
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
        ssExit << "whichMapTo: 0x" << std::hex << (int)this->whichMapTo << ", exitStartType: " << MapExitData::printExitStartType(this->exitStartType);
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
    static std::string printExitStartType(LevelSelectEnums::MapExitStartType exitType) {
        std::stringstream ss;
        ss << "0x" << std::hex << (uint16_t)exitType << " (";
        switch(exitType) {
            case LevelSelectEnums::MapExitStartType::PRESS_DOWN_PIPE: {
                ss << "Down Pipe";
                break;
            }
            case LevelSelectEnums::MapExitStartType::BLUE_DOOR: {
                ss << "Blue Door";
                break;
            }
            case LevelSelectEnums::MapExitStartType::AREA_TRIGGER_UP: {
                ss << "Area Trigger";
                break;
            }
            case LevelSelectEnums::MapExitStartType::WALK_TO_LEFT: {
                ss << "Walk Left";
                break;
            }
            default: {
                ss << "Unknown";
                break;
            }
        }
        ss << ")";
        return ss.str();
    };
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
        ssReturnEnter << "Entering animation: " << MapEntrance::printEntranceAnimation(this->enterMapAnimation) << ", ";
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
    // Entering animation
    static std::string printEntranceAnimation(LevelSelectEnums::MapEntranceAnimation enterAnim) {
        std::stringstream ss;
        ss << "0x" << std::hex << (uint16_t)enterAnim << " (";
        switch (enterAnim) {
            case LevelSelectEnums::MapEntranceAnimation::JUMP_IN_FROM_LEFT: {
                ss << "Jump from left";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::FLY_UP_LEFT: {
                ss << "Fly up leftwards";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_UPWARDS: {
                ss << "Pipe exit upwards";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_DOWNWARDS: {
                ss << "Pipe exit downwards";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::SLOW_FALL: {
                ss << "Slow fall";
                break;
            }
            default: {
                ss << "Unknown";
                break;
            }
        }
        ss << ")";
        return ss.str();
    };
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