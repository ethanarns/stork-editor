#pragma once

#include "../constants.h"
#include "../FsPacker.h"
#include "../utils.h"
#include "../Chartile.h"

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

class ImgbLayerData : public LevelData {
public:
    ImgbLayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "IMGB { records: 0x";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        // TODO
        return FsPacker::packInstruction(Constants::IMGB_MAGIC_NUM,result,false);
    };
    uint32_t magicNumber = Constants::IMGB_MAGIC_NUM;

    std::vector<Chartile> chartiles;
};

class MapCollisionData : public LevelData {
public:
    MapCollisionData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "COLZ { records: 0x";
        ss << std::hex << this->colData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        // 1:1 basically
        return FsPacker::packInstruction(Constants::COLZ_MAGIC_NUM,this->colData,true);
    };
    uint32_t magicNumber = Constants::COLZ_MAGIC_NUM;

    std::vector<uint8_t> colData;
};

class AnimatedMapData : public LevelData {
public:
    AnimatedMapData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "ANMZ { }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        // TODO //
        result = FsPacker::packInstruction(Constants::ANMZ_MAGIC_NUM,result,true);
        return result;
    };
    uint32_t magicNumber = Constants::ANMZ_MAGIC_NUM;

    uint8_t frameCount;
    uint8_t unknown1;
    uint16_t unknown2;
    uint16_t vramOffset;
    uint8_t unknown3;
    uint8_t unknown4;
    std::vector<uint8_t> frameTimes;
    std::vector<Chartile> chartiles;
};

class MapTilesData : public LevelData {
public:
    MapTilesData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "MPBZ { Palette Count: 0x";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        // TODO //
        result = FsPacker::packInstruction(Constants::MPBZ_MAGIC_NUM,result,true);
        return result;
    };
    uint32_t magicNumber = Constants::MPBZ_MAGIC_NUM;

    uint16_t tileOffset;
    uint16_t bottomTrim;
    std::vector<uint16_t> tileRenderData;
};

class LayerPaletteData : public LevelData {
public:
    LayerPaletteData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "LayerPaletteData { Palette Count: 0x";
        ss << std::hex << palettes.size() << " / ";
        ss << std::dec << palettes.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (auto it = this->palettes.cbegin(); it != this->palettes.cend(); it++) {
            for (uint pIndex = 0; pIndex < Constants::PALETTE_SIZE; pIndex++) {
                result.push_back((*it)->at(pIndex));
            }
        }
        result = FsPacker::packInstruction(Constants::PLTB_MAGIC_NUM,result,false);
        return result;
    };
    uint32_t magicNumber = Constants::PLTB_MAGIC_NUM;
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
    uint32_t magicNumber = Constants::SCEN_MAGIC_NUM;
    std::vector<LevelData*> subScenData;
};

class LevelGradientData : public LevelData {
public:
    LevelGradientData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "LevelGradientData { Subdata Count: ";
        ss << std::dec << this->subGradData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (size_t i = 0; i < this->subGradData.size(); i++) {
            auto subCompiled = this->subGradData.at(i)->compile();
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::GRAD_MAGIC_NUM,result,false);
        return result;
    };
    uint32_t magicNumber = Constants::GRAD_MAGIC_NUM;
    std::vector<LevelData*> subGradData;
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