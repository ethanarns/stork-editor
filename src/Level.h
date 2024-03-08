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

#endif