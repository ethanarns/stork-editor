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

struct ObjectPalette {
    QByteArray paletteData;
    uint32_t index;
    uint32_t address;
};

struct ObjectFile {
    std::string objectFileName;
    std::map<uint32_t,std::vector<uint8_t>> objectPixelTiles;
    std::map<uint32_t,ObjectPalette> objectPalettes;
};

#endif