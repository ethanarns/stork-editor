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
    uint16_t numEntranceOffsets; // 1-1: 0x0004
    // Pipe to coin running, shy guy pipe 1, shy guy pipe 2
    uint8_t numExitsInScene; // 1-1: 0x03
    uint8_t musicId; // 1-1: 0x09
    std::string mpdzFileNoExtension;
    // 8 zeroes
    std::vector<CscnEnterIntoMap> entrances;
    std::vector<CscnExitData> exits;
    std::string toString() {
        std::stringstream ssCscn;
        ssCscn << "CscnData { exit count: " << hex << this->numEntranceOffsets << ", ";
        ssCscn << "mpdz filename: " << this->mpdzFileNoExtension << " }";
        return ssCscn.str();
    }
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
    }
};

#endif