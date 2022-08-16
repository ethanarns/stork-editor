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

enum ExitLeaveType {
    OUT_OF_PIPE_UP = 0x01,
    OUT_OF_PIPE_DOWN = 0x00
};

struct CscnExitData {
    uint16_t exitLocationX;
    uint16_t exitLocationY;
    ExitStartType exitStartType;
    uint8_t whichMapTo;
    ExitLeaveType exitLeaveType;
    std::string toString() {
        std::stringstream ssExit;
        ssExit << "CscnExitData { target x/y: " << std::hex << this->exitLocationX;
        ssExit << "/" << std::hex << exitLocationY << ", ";
        ssExit << "whichMapTo: " << std::hex << (int)whichMapTo << ", exitStartType: " << std::hex << exitStartType;
        ssExit << ", exitLeaveType: " << std::hex << exitLeaveType << " }";
        return ssExit.str();
    }
};

// Reference: https://www.youtube.com/watch?v=Zb9jvSQg9sw
// Usually comes in as uint8_t, but this is stored as int
// To find these, break on 2013208, load 1-1, and edit 0231e307
enum Music {
    YOSHIS_ISLAND_DS = 0x02,
    SCORE = 0x07,
    FLOWER_FIELD = 0x09
};

struct CscnData {
    // (jump in, fly in from coin running level near growblock, shyguy pipe 1, shyguy pipe 2)
    uint16_t numEntranceOffsets; // 1-1: 0x0004
    // Pipe to coin running, shy guy pipe 1, shy guy pipe 2
    uint8_t numExitsInScene; // 1-1: 0x03
    uint8_t musicId; // 1-1: 0x09
    std::string mpdzFileNoExtension;
    // 8 zeroes
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