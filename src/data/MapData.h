#pragma once

#include "../constants.h"
#include "../FsPacker.h"
#include "../utils.h"
#include "../Chartile.h"
#include "../LevelObject.h"

#include <vector>
#include <string>
#include <sstream>
#include <QByteArray>

class LevelData {
public:
    virtual std::string toString() = 0;
    virtual std::vector<uint8_t> compile() = 0;
    virtual uint32_t getMagic() { return 0; };
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
        ss << "  Color Mode (Maybe): 0x" << this->colorMode << std::endl;
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
    uint32_t getMagic() { return Constants::INFO_MAGIC_NUM; }
    // Variables
    uint16_t layerWidth;
    uint16_t layerHeight;
    uint32_t bgYoffset;
    uint32_t xScrollOffset;
    uint32_t yScrollOffset;
    uint8_t whichBackground;
    uint8_t layerOrder;
    // Not related to which other SCEN sub-datas present
    // Switching from 4 to 2 on first 1-1 level causes tiles to fuck up. Vertical lines. Maybe compression related?
    // Setting that to 3 makes only SOME tiles messed up
    // Setting it to 1 makes the background stop being transparent, now it's all opaque diagonal lines. Rest screwed up too
    // Only 1-4 available. Definitely tile related
    uint8_t unkThird;
    uint8_t baseBlockMaybe;
    BgColorMode colorMode;
    std::string imbzFilename;
};

class LevelObjectData : public LevelData {
public:
    LevelObjectData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "LevelObjectData(SETD) { records: 0x";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        // TODO
        return FsPacker::packInstruction(Constants::SETD_MAGIC_NUM,result,false);
    };
    uint32_t getMagic() { return Constants::SETD_MAGIC_NUM; }

    std::vector<LevelObject*> levelObjects;
private:
    uint32_t uuidIndex = 1;
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
    uint32_t getMagic() { return Constants::IMGB_MAGIC_NUM; }

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
    uint32_t getMagic() { return Constants::COLZ_MAGIC_NUM; }

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
    uint32_t getMagic() { return Constants::ANMZ_MAGIC_NUM; }

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
    MapTilesData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, LayerData* parent);
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
    uint32_t getMagic() { return Constants::MPBZ_MAGIC_NUM; }

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
    uint32_t getMagic() { return Constants::PLTB_MAGIC_NUM; }

    std::vector<QByteArray*> palettes;
};

// SCEN
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
    uint32_t getMagic() { return Constants::SCEN_MAGIC_NUM; }

    // A mirror of the VRAM set aside for this BG
    std::map<uint32_t,Chartile> pixelTiles;

    ScenInfoData* getInfo();
    LayerPaletteData* getPalette();
    LevelData* getFirstDataByMagic(uint32_t magicNumber, bool silentFail = false);
    std::vector<uint16_t> getPreRenderData();
private:
    std::vector<LevelData*> subScenData;
    // VRAM index (temporary)
    uint32_t pixelTileIndex;
};

// GRAD
class LevelGradientData : public LevelData {
public:
    LevelGradientData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "LevelGradientData { ";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        // TODO
        result = FsPacker::packInstruction(Constants::GRAD_MAGIC_NUM,result,false);
        return result;
    };
    uint32_t getMagic() { return Constants::GRAD_MAGIC_NUM; }
    // Every single time, it's GINF then GCOL. No need to loop
    // GINF //
    uint8_t unknown1;
    uint8_t unknown2;
    uint8_t unknown3;
    uint8_t unknown4;
    uint16_t unknown5;
    uint8_t unknown6;
    uint8_t unknown7;
    uint32_t yOffset;
    // GCOL //
    std::vector<uint16_t> colors;
};

class MapData : public LevelData {
public:
    MapData(std::vector<uint8_t> mpdzBytes, bool compressed = true);
    uint32_t getMagic() { return Constants::MPDZ_MAGIC_NUM; }
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
    LayerData* getScenByBg(uint8_t bg);
    std::vector<LevelObject*> getAllLevelObjects();
    LevelObject* getLevelObjectByUuid(uint32_t uuid);
    uint32_t getGreatestCanvasHeight();
    uint32_t getGreatestCanvasWidth();
    /**
     * @brief Collision tiles, each taking up a full 16x16 tile. Also handles 
     * static normal coins for some reason
     */
    std::vector<uint8_t> getCollisionArray();
private:    
    std::vector<LevelData*> subData;
    LevelData* getFirstDataByMagic(uint32_t magicNumber);
};