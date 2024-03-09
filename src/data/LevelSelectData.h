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
        WALK_TO_RIGHT = 0x00,
        WALK_TO_LEFT = 0x01,
        TOUCH_PIPE_UP = 0x02, // Touching it does the pipe warp up, no button needed
        PRESS_UP_PIPE = 0x03, // Requires pressing up key
        PRESS_DOWN_PIPE = 0x04, // Use on 1-1 a lot
        BLUE_DOOR = 0x05, // Good for forts
        BLUE_DOOR_LOCKED = 0x06, // Requires a key
        BOSS_DOOR = 0x07, // Red, only for boss rooms
        UNKNOWN_1 = 0x08,// 0x8 I could not get to trigger when replacing down pipe on 1-1
        WALK_RIGHT_QUIT_MAP = 0x09, // Quits the map when you walk right??
        PRESS_RIGHT_PIPE = 0x0A,
        PRESS_LEFT_PIPE = 0x0B,
        AREA_TRIGGER_PIPE = 0x0C, // Simply jumping into the area triggers the exit
        UNKNOWN_2 = 0x0D, // 0x0D does not seem to trigger with 1-1 pipe replace
        AREA_TRIGGER_SILENT = 0x0E, // Silently changes maps
        // 0x0F I could not get to trigger
        // 0x10 neither. 0xE might be the last one
    };

    enum MapEntranceAnimation {
        SPAWN_STATIC_RIGHT = 0x00, // If first map entrance, this is jump in from left. Pretty much always uses this
        SPAWN_STATIC_LEFT = 0x01,  // If first map entrance, this is jump in from right. Unsure if used in base game
        WALK_OUT_RIGHT = 0x02,
        WALK_OUT_LEFT = 0x03,
        SLOW_FALL_FACE_RIGHT = 0x04, // Slowly fall for a bit, then gravity resumes. "Big pipes" or ground holes
        SLOW_FALL_FACE_LEFT = 0x05,
        OUT_OF_PIPE_UPWARDS_SILENT_RIGHT = 0x06, // Pipe animation shown, but no sound places
        OUT_OF_PIPE_UPWARDS_SILENT_LEFT = 0x07,
        FLY_UP_RIGHT = 0x08, // Being shot up from underground, or going up to a cloud area usually
        FLY_UP_LEFT = 0x09,
        LOCKED_BLUE_DOOR_RIGHT = 0x0A,
        OUT_OF_PIPE_UPWARDS_RIGHTFACE = 0x0B,
        OUT_OF_PIPE_DOWNWARDS_RIGHTFACE = 0x0C,
        OUT_OF_PIPE_UPWARDS_LEFTFACE = 0x0D,
        OUT_OF_PIPE_DOWNWARDS_LEFTFACE = 0x0E,
        OUT_OF_PIPE_RIGHTWARDS = 0x0F,
        OUT_OF_PIPE_LEFTWARDS = 0x10,
        LOCKED_BLUE_DOOR_LEFT = 0x11,
        YOSHI_IS_INVISIBLE = 0x012, // Special case? Investigate in code
        // Then 0x13 and 0x14 is spawn static left.. broken past 0x11 or 0x12 probably
    };

    enum StartingDsScreen {
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
                ss << "Down Button Pipe";
                break;
            }
            case LevelSelectEnums::MapExitStartType::PRESS_UP_PIPE: {
                ss << "Up Button Pipe";
                break;
            }
            case LevelSelectEnums::MapExitStartType::PRESS_LEFT_PIPE: {
                ss << "Left Button Pipe";
                break;
            }
            case LevelSelectEnums::MapExitStartType::PRESS_RIGHT_PIPE: {
                ss << "Right Button Pipe";
                break;
            }
            case LevelSelectEnums::MapExitStartType::BLUE_DOOR: {
                ss << "Blue Door";
                break;
            }
            case LevelSelectEnums::MapExitStartType::BLUE_DOOR_LOCKED: {
                ss << "Blue Door (Needs key)";
                break;
            }
            case LevelSelectEnums::MapExitStartType::BOSS_DOOR: {
                ss << "Boss Door";
                break;
            }
            case LevelSelectEnums::MapExitStartType::AREA_TRIGGER_PIPE: {
                ss << "Area Trigger (Pipe sound)";
                break;
            }
            case LevelSelectEnums::MapExitStartType::AREA_TRIGGER_SILENT: {
                ss << "Area Trigger (Silent)";
                break;
            }
            case LevelSelectEnums::MapExitStartType::WALK_TO_LEFT: {
                ss << "Walk Left";
                break;
            }
            case LevelSelectEnums::MapExitStartType::WALK_TO_RIGHT: {
                ss << "Walk Right";
                break;
            }
            case LevelSelectEnums::MapExitStartType::WALK_RIGHT_QUIT_MAP: {
                ss << "Walk Right (Quit to level select)";
                break;
            }
            case LevelSelectEnums::MapExitStartType::TOUCH_PIPE_UP: {
                ss << "Automatic Up Pipe";
                break;
            }
            case LevelSelectEnums::MapExitStartType::UNKNOWN_1: {
                ss << "Unknown 1";
                break;
            }
            case LevelSelectEnums::MapExitStartType::UNKNOWN_2: {
                ss << "Unknown 2";
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
    LevelSelectEnums::StartingDsScreen whichDsScreen;
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
            case LevelSelectEnums::MapEntranceAnimation::SPAWN_STATIC_RIGHT: {
                ss << "Spawn static (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::SPAWN_STATIC_LEFT: {
                ss << "Spawn static (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::FLY_UP_LEFT: {
                ss << "Shot upwards (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::FLY_UP_RIGHT: {
                ss << "Shot upwards (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_UPWARDS_RIGHTFACE: {
                ss << "Pipe exit upwards (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_UPWARDS_LEFTFACE: {
                ss << "Pipe exit upwards (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_DOWNWARDS_RIGHTFACE: {
                ss << "Pipe exit down (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_DOWNWARDS_LEFTFACE: {
                ss << "Pipe exit down (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::SLOW_FALL_FACE_RIGHT: {
                ss << "Slow fall (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::SLOW_FALL_FACE_LEFT: {
                ss << "Slow fall (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_UPWARDS_SILENT_LEFT: {
                ss << "Silent pipe upwards (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_UPWARDS_SILENT_RIGHT: {
                ss << "Silent pipe upwards (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::WALK_OUT_LEFT: {
                ss << "Walk out left";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::WALK_OUT_RIGHT: {
                ss << "Walk out right";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::LOCKED_BLUE_DOOR_RIGHT: {
                ss << "Locked blue door (Right)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::LOCKED_BLUE_DOOR_LEFT: {
                ss << "Locked blue door (Left)";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_LEFTWARDS: {
                ss << "Walk left out of pipe";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::OUT_OF_PIPE_RIGHTWARDS: {
                ss << "Walk right out of pipe";
                break;
            }
            case LevelSelectEnums::MapEntranceAnimation::YOSHI_IS_INVISIBLE: {
                ss << "Spawn Yoshi invisible";
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
    LevelMetadata* getLevelByMpdz(std::string mpdzFilename);
};