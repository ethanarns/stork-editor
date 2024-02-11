#pragma once

#include "../constants.h"
#include "../FsPacker.h"
#include "../utils.h"

#include <vector>
#include <string>
#include <sstream>
#include <QByteArray>

class LevelData {
public:
    virtual std::string toString() = 0;
    virtual std::vector<uint8_t> compile() = 0;
    uint32_t magicNumber = 0;
};

class LayerPaletteData : public LevelData {
public:
    LayerPaletteData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "LayerPaletteData { Palette Count: ";
        ss << std::dec << palettes.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        // for (size_t i = 0; i < subScenData.size(); i++) {
        //     auto subCompiled = subScenData.at(i)->compile();
        //     YUtils::appendVector(result,subCompiled);
        // }
        result = FsPacker::packInstruction(Constants::PLTB_MAGIC_NUM,result);
        return result;
    };
    uint32_t magicNumber = 0;
    std::vector<QByteArray*> palettes;
};

class ScenInfoData : public LevelData {
public:
    ScenInfoData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    virtual std::string toString() {
        std::stringstream ss;
        ss << "ScenInfoData {" << std::hex << std::endl;
        ss << "  Layer Width: 0x" << this->layerWidth << std::endl;
        ss << "  Layer Height: 0x" << this->layerHeight << std::endl;
        ss << "  BG Y Offset: 0x" << this->bgYoffset << std::endl;
        ss << "  X Scroll Offset: 0x" << this->xScrollOffset << std::endl;
        ss << "  Y Scroll Offset: 0x" << this->yScrollOffset << std::endl;
        ss << "  Background Index: 0x" << (uint16_t)this->whichBackground << std::endl;
        ss << "  Layer Order: 0x" << (uint16_t)this->layerOrder << std::endl;
        ss << "  Unknown 3rd: 0x" << (uint16_t)this->unkThird << std::endl;
        ss << "  Base Block (Maybe): 0x" << (uint16_t)this->baseBlockMaybe << std::endl;
        ss << "  Color Mode (Maybe): 0x" << this->colorModeMaybe << std::endl;
        if (!this->imbzFilename.empty()) {
            ss << "  IMBZ File: " << this->imbzFilename;
        }
        ss << "}";
        return ss.str();
    };
    virtual std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        result = FsPacker::packInstruction(Constants::INFO_MAGIC_NUM,result,false);
        return result;
    };
    uint32_t magicNumber = Constants::INFO_MAGIC_NUM;
    // Variables
    uint16_t layerWidth;
    uint16_t layerHeight;
    uint32_t bgYoffset;
    uint32_t xScrollOffset;
    uint32_t yScrollOffset;
    uint8_t whichBackground;
    uint8_t layerOrder;
    uint8_t unkThird;
    uint8_t baseBlockMaybe;
    uint32_t colorModeMaybe;
    std::string imbzFilename;
};

class LayerData : public LevelData {
public:
    LayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "LayerData { Subdata Count: ";
        ss << std::dec << subScenData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (size_t i = 0; i < subScenData.size(); i++) {
            auto subCompiled = subScenData.at(i)->compile();
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::SCEN_MAGIC_NUM,result);
        return result;
    };
    uint32_t magicNumber = 0;
    std::vector<LevelData*> subScenData;
};

class MapData : public LevelData {
public:
    MapData(std::vector<uint8_t> mpdzBytes, bool compressed = true);
    uint32_t magicNumber = Constants::MPDZ_MAGIC_NUM;
    std::string toString() {
        std::stringstream ss;
        ss << "MapData { Subdata Count: ";
        ss << std::dec << subData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (size_t i = 0; i < subData.size(); i++) {
            auto subCompiled = subData.at(i)->compile();
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::MPDZ_MAGIC_NUM,result);
        return result;
    };
    // Others
    LayerData* getScenByBg(int bg);
private:    
    std::vector<LevelData*> subData;
};