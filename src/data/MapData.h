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

// Forward declaration
class LayerData;

class LevelData {
public:
    virtual std::string toString() = 0;
    virtual std::vector<uint8_t> compile() = 0;
    virtual uint32_t getMagic() { return 0; };
    virtual ~LevelData() { /* Do stuff */ };
};

// INFO
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

// SETD
class LevelObjectData : public LevelData {
public:
    LevelObjectData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    ~LevelObjectData();
    std::string toString() {
        std::stringstream ss;
        ss << "LevelObjectData(SETD) { records: 0x";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (auto it = this->levelObjects.begin(); it != this->levelObjects.end(); it++) {
            LevelObject obj = *(*it);
            auto objVec = YUtils::compileObject(obj);
            YUtils::appendVector(result,objVec);
        }
        return FsPacker::packInstruction(Constants::SETD_MAGIC_NUM,result,false);
    };
    uint32_t getMagic() { return Constants::SETD_MAGIC_NUM; }

    std::vector<LevelObject*> levelObjects;
private:
    uint32_t uuidIndex = 1;
};

// IMGB
class ImgbLayerData : public LevelData {
public:
    ImgbLayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    ~ImgbLayerData();
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

// COLZ
class MapCollisionData : public LevelData {
public:
    MapCollisionData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    ~MapCollisionData();
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

// ANMZ
class AnimatedMapData : public LevelData {
public:
    AnimatedMapData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, BgColorMode &colorMode);
    ~AnimatedMapData();
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

// MPBZ
class MapTilesData : public LevelData {
public:
    MapTilesData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, LayerData* parent);
    ~MapTilesData();
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

// PLTB
class LayerPaletteData : public LevelData {
public:
    LayerPaletteData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    ~LayerPaletteData();
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
    ~LayerData();
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
    std::map<uint32_t,Chartile> getVramChartiles();

    ScenInfoData* getInfo();
    LayerPaletteData* getPalette();
    LevelData* getFirstDataByMagic(uint32_t magicNumber, bool silentFail = false);
    /// @brief Return the two-byte map tile vector representing the VRAM
    /// @return Vector of uint16s representing ROM map tiles (see Map Address in No$GBA)
    std::vector<uint16_t> getMapTiles();
    std::vector<Chartile> parseImbzFromFile(std::string filename_noExt, BgColorMode bgColMode = BgColorMode::MODE_16);
private:
    std::vector<LevelData*> subScenData;

    std::string cachedImbzFilenameNoExt;
    std::vector<Chartile> cachedImbzTileData;
    std::map<uint32_t,Chartile> cachedVramTiles;
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
        // GINF
        std::vector<uint8_t> result = YUtils::uint32toVec(Constants::GINF_MAGIC_NUM);
        uint32_t len = 12;
        auto lenVec = YUtils::uint32toVec(len);
        YUtils::appendVector(result,lenVec);
        result.push_back(this->unknown1);
        result.push_back(this->unknown2);
        result.push_back(this->unknown3);
        result.push_back(this->unknown4);
        auto unk5 = YUtils::uint16toVec(this->unknown5);
        YUtils::appendVector(result,unk5);
        result.push_back(this->unknown6);
        result.push_back(this->unknown7);
        auto yOffset = YUtils::uint32toVec(this->yOffset);
        YUtils::appendVector(result,yOffset);
        // GCOL
        auto gcol = YUtils::uint32toVec(Constants::GCOL_MAGIC_NUM);
        YUtils::appendVector(result,gcol);
        uint32_t gcolLength = this->colors.size() * 2;
        auto gcolLengthVec = YUtils::uint32toVec(gcolLength);
        YUtils::appendVector(result,gcolLengthVec);
        std::cout << "b" << std::endl;
        for (auto it = this->colors.begin(); it != this->colors.end(); it++) {
            auto curColor = YUtils::uint16toVec(*it);
            YUtils::appendVector(result,curColor);
        }
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

// MPDZ
class MapData : public LevelData {
public:
    MapData(std::vector<uint8_t> mpdzBytes, bool compressed = true);
    ~MapData();
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
            std::cout << "Compiling subData: " << std::hex << subData.at(i)->getMagic() << std::endl;
            auto subCompiled = subData.at(i)->compile();
            std::cout << "Compiled" << std::endl;
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::MPDZ_MAGIC_NUM,result,true);
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
    /**
     * Get the layer width of which SCEN has COLZ data
    */
    uint32_t getCollisionCanvasWidth();

    std::vector<QByteArray*> getBackgroundPalettes(QByteArray universalPalette);
    /// @brief Wipes the cached of BGP
    /// @return true if there was anything to wipe, false if nothing was wiped
    bool wipeBGPcache();

    QByteArray getLayerOrder();
    bool wipeLayerOrderCache();

    LevelData* getFirstDataByMagic(uint32_t magicNumber);
private:    
    std::vector<LevelData*> subData;
    std::vector<QByteArray*> bgPalleteRamCache;
    QByteArray layerOrderCache;
};