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
#include <map>

#include "utils.h"
#include "FsPacker.h"
#include "constants.h"
#include "Chartile.h"
#include "compression.h"

class Instruction {
public:
    virtual std::string toString() = 0;
    virtual std::vector<uint8_t> compile() = 0;
    uint32_t magicNum = 0;
};

struct ObjectPalette {
    QByteArray paletteData;
    uint32_t index;
    uint32_t address;
};

/**
 * @brief MPDZ file
 */
struct MapFile : public Instruction {
    uint32_t magicNum = Constants::MPDZ_MAGIC_NUM;
    std::vector<Instruction*> majorInstructions;

    std::string toString() {
        return "MapFile {}";
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        result = FsPacker::packInstruction(Constants::MPDZ_MAGIC_NUM,result);
        return result;
    };
    ~MapFile() {
        // This trick deletes all the data's memory in the vector
        std::vector<Instruction*>().swap(this->majorInstructions);
    }
};

struct ObjectFile {
    std::string objectFileName;
    std::map<uint32_t,std::vector<uint8_t>> objectPixelTiles;
    std::map<uint32_t,ObjectPalette> objectPalettes;
};

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

#endif