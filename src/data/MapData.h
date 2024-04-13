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
class ScenInfoData;

class LevelData {
public:
    virtual std::string toString() = 0;
    virtual std::vector<uint8_t> compile(ScenInfoData &info) = 0;
    virtual uint32_t getMagic() { return 0; };
    virtual ~LevelData() { /* Do stuff */ };
};

// INFO
class ScenInfoData : public LevelData {
public:
    ScenInfoData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    ScenInfoData();
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
        ss << "  Char Base Block: 0x" << (uint16_t)this->charBaseBlock << std::endl;
        ss << "  Screen Base Block: 0x" << (uint16_t)this->screenBaseBlock << std::endl;
        ss << "  Color Mode (Maybe): 0x" << this->colorMode << std::endl;
        if (!this->imbzFilename.empty()) {
            ss << "  IMBZ File: " << this->imbzFilename;
        }
        ss << "}";
        return ss.str();
    };
    virtual std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        auto layerWidth = YUtils::uint16toVec(this->layerWidth);
        YUtils::appendVector(result,layerWidth);
        auto layerHeight = YUtils::uint16toVec(this->layerHeight);
        YUtils::appendVector(result,layerHeight);
        auto bgYoffset = YUtils::uint32toVec(this->bgYoffset);
        YUtils::appendVector(result,bgYoffset);
        auto xScrollOffset = YUtils::uint32toVec(this->xScrollOffset);
        YUtils::appendVector(result,xScrollOffset);
        auto yScrollOffset = YUtils::uint32toVec(this->yScrollOffset);
        YUtils::appendVector(result,yScrollOffset);
        // Single bytes
        result.push_back(this->whichBackground);
        result.push_back(this->layerOrder);
        result.push_back(this->charBaseBlock);
        result.push_back(this->screenBaseBlock);
        auto colorMode = YUtils::uint32toVec((uint32_t)this->colorMode);
        YUtils::appendVector(result,colorMode);
        if (!this->imbzFilename.empty()) {
            auto imbzFilenameVec = YUtils::stringToVector(this->imbzFilename);
            YUtils::appendVector(result,imbzFilenameVec);
            while (result.size() % 4 != 0) {
                result.push_back(0x0);
            }
        }
        return FsPacker::packInstruction(Constants::INFO_MAGIC_NUM,result,false);
    };
    uint32_t getMagic() { return Constants::INFO_MAGIC_NUM; }
    // Variables
    uint16_t layerWidth;
    uint16_t layerHeight;
    uint32_t bgYoffset;
    uint32_t xScrollOffset;
    uint32_t yScrollOffset;
    uint8_t whichBackground;
    uint8_t layerOrder; // Usually matches whichBg
    uint8_t charBaseBlock; // Where in VRAM stored https://mtheall.com/vram.html, separate!
    uint8_t screenBaseBlock; // Likely unimportant, map tiles are definitely not shared
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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        for (auto it = this->chartiles.begin(); it != this->chartiles.end(); it++) {
            auto tiles = it->tiles;
            for (int i = 0; i < tiles.size(); i += 2) {
                auto tile0 = tiles.at(i);
                auto tile1 = tiles.at(i+1);
                result.push_back((tile1 << 4) + tile0);
            }
        }
        return FsPacker::packInstruction(Constants::IMGB_MAGIC_NUM,result,false);
    };
    uint32_t getMagic() { return Constants::IMGB_MAGIC_NUM; }

    std::vector<Chartile> chartiles;
};

// IMBZ
class ImbzLayerData : public LevelData {
public:
    ImbzLayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, BgColorMode colorMode);
    ~ImbzLayerData();
    std::string toString() {
        std::stringstream ss;
        ss << "IMBZ { records: 0x";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        for (auto it = this->chartiles.begin(); it != this->chartiles.end(); it++) {
            auto tiles = it->tiles;
            for (int i = 0; i < tiles.size(); i += 2) {
                auto tile0 = tiles.at(i);
                auto tile1 = tiles.at(i+1);
                result.push_back((tile1 << 4) + tile0);
            }
        }
        return FsPacker::packInstruction(Constants::IMBZ_MAGIC_NUM,result,true);
    };
    uint32_t getMagic() { return Constants::IMBZ_MAGIC_NUM; }

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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        result.push_back(this->frameCount);
        result.push_back(this->unknown1);
        auto unknown2 = YUtils::uint16toVec(this->unknown2);
        YUtils::appendVector(result, unknown2);
        auto vramOffset = YUtils::uint16toVec(this->vramOffset);
        YUtils::appendVector(result, vramOffset);
        result.push_back(this->unknown3);
        result.push_back(this->unknown4);
        YUtils::appendVector(result,this->frameTimes);
        while(result.size() % 4 != 0) {
            result.push_back(0x0);
        }
        // Now do it like IMGB
        for (auto it = this->chartiles.begin(); it != this->chartiles.end(); it++) {
            auto tiles = it->tiles;
            BgColorMode colMode = it->bgColMode;
            if (colMode == BgColorMode::MODE_16) {
                for (int i = 0; i < tiles.size(); i += 2) {
                    auto tile0 = tiles.at(i);
                    auto tile1 = tiles.at(i+1);
                    result.push_back((tile1 << 4) + tile0);
                }
            } else {
                //256 mode
                for (int i = 0; i < tiles.size(); i++) {
                    result.push_back(tiles.at(i));
                }
            }
        }
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

// ALPH
class AlphaData : public LevelData {
public:
    AlphaData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    std::string toString() {
        std::stringstream ss;
        ss << "ALPH { UNHANDLED";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        result.push_back(this->byte1);
        result.push_back(this->byte2);
        result.push_back(this->byte3);
        result.push_back(this->byte4);
        return FsPacker::packInstruction(Constants::ALPH_MAGIC_NUM,result,false);
    };
    uint32_t getMagic() { return Constants::ALPH_MAGIC_NUM; }

    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        std::vector<uint8_t> result;
        uint32_t startOffset = 0;
        if (this->tileOffset > 0 || this->bottomTrim > 0) {
            result.push_back(0xff);
            result.push_back(0xff);
            auto tileOffsetVec = YUtils::uint16toVec(this->tileOffset);
            YUtils::appendVector(result,tileOffsetVec);
            auto bottomTrimVec = YUtils::uint16toVec(this->bottomTrim);
            YUtils::appendVector(result,bottomTrimVec);
            startOffset = this->tileOffset * info.layerWidth;
            // Padding header built
        }
        bool isColorMode16 = info.colorMode == BgColorMode::MODE_16;
        bool isColorModeUnknown = info.colorMode == BgColorMode::MODE_UNKNOWN;
        for (uint i = startOffset; i < this->mapTiles.size(); i++) {
            uint16_t curShort = this->mapTiles.at(i).compile();
            if (isColorMode16 || isColorModeUnknown) {
                curShort -= 0x1000;
            }
            auto vec = YUtils::uint16toVec(curShort);
            YUtils::appendVector(result,vec);
        }
        result = FsPacker::packInstruction(Constants::MPBZ_MAGIC_NUM,result,true);
        return result;
    };
    uint32_t getMagic() { return Constants::MPBZ_MAGIC_NUM; }

    uint16_t tileOffset;
    uint16_t bottomTrim;
    std::vector<MapTileRecordData> mapTiles;
};

// PLTB
class LayerPaletteData : public LevelData {
public:
    LayerPaletteData();
    LayerPaletteData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, BgColorMode colMode);
    ~LayerPaletteData();
    std::string toString() {
        std::stringstream ss;
        ss << "LayerPaletteData { Palette Count: 0x";
        ss << std::hex << palettes.size() << " / ";
        ss << std::dec << palettes.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        std::vector<uint8_t> result;
        if (info.colorMode == BgColorMode::MODE_16 || info.colorMode == BgColorMode::MODE_UNKNOWN) {
            for (auto it = this->palettes.cbegin(); it != this->palettes.cend(); it++) {
                for (uint pIndex = 0; pIndex < Constants::PALETTE_SIZE; pIndex++) {
                    result.push_back(it->at(pIndex));
                }
            }
        } else if (info.colorMode == BgColorMode::MODE_256) { // Extended mode/256
            for (uint eIndex = 0; eIndex < this->extendedPalette.size(); eIndex++) {
                result.push_back(this->extendedPalette.at(eIndex));
            }
        } else {
            YUtils::printDebug("Unknown color mode in LayerPaletteData",DebugType::ERROR);
        }
        result = FsPacker::packInstruction(Constants::PLTB_MAGIC_NUM,result,false);
        return result;
    };
    uint32_t getMagic() { return Constants::PLTB_MAGIC_NUM; }

    std::vector<QByteArray> palettes;
    QByteArray extendedPalette;
    uint32_t bgOffset;
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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        for (size_t i = 0; i < subScenData.size(); i++) {
            auto tempInfo = this->getInfo();
            auto subCompiled = subScenData.at(i)->compile(*tempInfo);
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::SCEN_MAGIC_NUM,result);
        return result;
    };
    uint32_t getMagic() { return Constants::SCEN_MAGIC_NUM; }

    ScenInfoData* getInfo();
    LayerPaletteData* getPalette();
    LevelData* getFirstDataByMagic(uint32_t magicNumber, bool silentFail = false);
    /// @brief Return the two-byte map tile vector representing the VRAM
    /// @return Vector of uint16s representing ROM map tiles (see Map Address in No$GBA)
    std::vector<MapTileRecordData> getMapTiles();
    std::vector<Chartile> parseImbzFromFile(std::string filename_noExt, BgColorMode bgColMode = BgColorMode::MODE_16);

    uint32_t magicOfChartilesSource = 0;
    bool hasAnmzChartiles = false;
    std::vector<LevelData*> subScenData;
private:

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
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
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

struct TriggerBox {
    uint16_t leftX;
    uint16_t topY;
    uint16_t rightX;
    uint16_t bottomY;
    // This is only for rendering purposes
    uint32_t uuid;
    std::string toString() {
        std::stringstream ss;
        ss << "TriggerBox { UUID: 0x";
        ss << std::hex << this->uuid;
        ss << ", leftX: " << this->leftX;
        ss << ", topY: " << this->topY;
        ss << ", rightX: " << this->rightX;
        ss << ", bottomY: " << this->bottomY;
        ss << " }";
        return ss.str();
    };
};

// AREA
class TriggerBoxData : public LevelData {
public:
    TriggerBoxData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    uint32_t getMagic() { return Constants::AREA_MAGIC_NUM; }
    std::string toString() {
        std::stringstream ss;
        ss << "TriggerBoxData { TriggerBox count: ";
        ss << std::dec << this->triggers.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        for (auto it = this->triggers.begin(); it != triggers.end(); it++) {
            auto trigger = *it;
            auto leftx = trigger->leftX;
            auto leftxVec = YUtils::uint16toVec(leftx);
            YUtils::appendVector(result,leftxVec);

            auto topy = trigger->topY;
            auto topyVec = YUtils::uint16toVec(topy);
            YUtils::appendVector(result,topyVec);

            auto rightx = trigger->rightX;
            auto rightxVec = YUtils::uint16toVec(rightx);
            YUtils::appendVector(result,rightxVec);

            auto bottomy = trigger->bottomY;
            auto bottomyVec = YUtils::uint16toVec(bottomy);
            YUtils::appendVector(result,bottomyVec);
        }
        result = FsPacker::packInstruction(Constants::AREA_MAGIC_NUM,result,false);
        return result;
    }

    std::vector<TriggerBox*> triggers;
};

struct PathSection {
    // Can also be the "final angle" of a path
    uint16_t angle;
    // Can be 0x0000, indicating the end of a path
    uint16_t distance;
    // X Position starts at, or if last, end X
    uint32_t xFine;
    // Y Position starts at, or if last, end Y
    uint32_t yFine;
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        auto angleVec = YUtils::uint16toVec(this->angle);
        auto distanceVec = YUtils::uint16toVec(this->distance);
        auto xFineVec = YUtils::uint32toVec(this->xFine);
        auto yFineVec = YUtils::uint32toVec(this->yFine);
        YUtils::appendVector(result,angleVec);
        YUtils::appendVector(result,distanceVec);
        YUtils::appendVector(result,xFineVec);
        YUtils::appendVector(result,yFineVec);
        // do not "pack"
        return result;
    };
};

// PATH
class PathData : public LevelData {
public:
    PathData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    uint32_t getMagic() { return Constants::PATH_MAGIC_NUM; }
    std::string toString() {
        std::stringstream ss;
        ss << "PathData { Path count: ???";
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        uint32_t pathCount = this->paths.size();
        auto pathCountVec = YUtils::uint32toVec(pathCount);
        YUtils::appendVector(result,pathCountVec);
        for (uint primaryPathIndex = 0; primaryPathIndex < pathCount; primaryPathIndex++) {
            uint subPathRecordCount = this->paths.at(primaryPathIndex).size();
            for (uint subPathIndex = 0; subPathIndex < subPathRecordCount; subPathIndex++) {
                auto comp = this->paths.at(primaryPathIndex).at(subPathIndex)->compile();
                YUtils::appendVector(result,comp);
            }
        }
        result = FsPacker::packInstruction(Constants::PATH_MAGIC_NUM,result,false);
        return result;
    }
    std::vector<std::vector<PathSection*>> paths;
};

// PLAN
class PaletteAnimationData : public LevelData {
public:
    PaletteAnimationData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    uint32_t getMagic() { return Constants::PLAN_MAGIC_NUM; }
    std::string toString() {
        std::stringstream ss;
        ss << "PaletteAnimationData { Palette Start Index: ...";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        result.push_back(palAnimStart);
        result.push_back(palAnimStop);
        result.push_back(frameHoldTime);
        result.push_back(frameCount);
        for (auto it = colors.begin(); it != colors.end(); it++) {
            auto colorVec = YUtils::uint16toVec(*it);
            YUtils::appendVector(result,colorVec);
        }
        while (result.size() % 4 != 0) {
            result.push_back(0x00);
        }
        result = FsPacker::packInstruction(Constants::PLAN_MAGIC_NUM,result,false);
        return result;
    };
    uint8_t palAnimStart;
    uint8_t palAnimStop;
    uint8_t frameHoldTime;
    uint8_t frameCount;
    std::vector<uint16_t> colors;
};

// BLKZ
class SoftRockBackdrop : public LevelData {
public:
    SoftRockBackdrop(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    uint32_t getMagic() { return Constants::BLKZ_MAGIC_NUM; }
    std::string toString() {
        std::stringstream ss;
        ss << "SoftRockBackdrop { todo ...";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result;
        auto xOff = YUtils::uint16toVec(this->xOffset);
        YUtils::appendVector(result,xOff);
        auto yOff = YUtils::uint16toVec(this->yOffset);
        YUtils::appendVector(result,yOff);
        auto w = YUtils::uint16toVec(this->width);
        YUtils::appendVector(result,w);
        auto h = YUtils::uint16toVec(this->height);
        YUtils::appendVector(result,h);
        for (auto it = this->mapTiles.begin(); it != this->mapTiles.end(); it++) {
            auto us = it->compile();
            auto usv = YUtils::uint16toVec(us);
            YUtils::appendVector(result,usv);
        }
        result = FsPacker::packInstruction(Constants::BLKZ_MAGIC_NUM,result,true);
        return result;
    };
    uint16_t xOffset;
    uint16_t yOffset;
    uint16_t width;
    uint16_t height;
    std::vector<MapTileRecordData> mapTiles;
};

// BRAK
class SoftRockSiding : public LevelData {
public:
    SoftRockSiding(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop);
    uint32_t getMagic() { return Constants::BRAK_MAGIC_NUM; }
    std::string toString() {
        std::stringstream ss;
        ss << "SoftRockSiding? { todo ...";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        Q_UNUSED(info);
        std::vector<uint8_t> result = this->bytes;
        result = FsPacker::packInstruction(Constants::BRAK_MAGIC_NUM,result,false);
        return result;
    };
    std::vector<uint8_t> bytes;
};

// MPDZ
class MapData : public LevelData {
public:
    MapData(std::vector<uint8_t> mpdzBytes, bool compressed, QByteArray backgroundPalettesRef[0x20]);
    ~MapData();
    uint32_t getMagic() { return Constants::MPDZ_MAGIC_NUM; }
    std::string toString() {
        std::stringstream ss;
        ss << "MapData { Subdata Count: ";
        ss << std::dec << subData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile(ScenInfoData &info) {
        std::vector<uint8_t> result;
        for (size_t i = 0; i < subData.size(); i++) {
            auto subCompiled = subData.at(i)->compile(info);
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::MPDZ_MAGIC_NUM,result,false);
        return result;
    };
    // Others
    LayerData* getScenByBg(uint8_t bg, bool silentFail = false);
    std::vector<LevelObject*> getAllLevelObjects();
    LevelObject* getLevelObjectByUuid(uint32_t uuid);
    uint32_t getGreatestCanvasHeight();
    uint32_t getGreatestCanvasWidth();
    /**
     * @brief Collision tiles, each taking up a full 16x16 tile. Also handles 
     * static normal coins for some reason
     */
    MapCollisionData* getCollisionData();

    /**
     * Get the layer width of which SCEN has COLZ data
    */
    uint32_t getCollisionCanvasWidth();

    std::vector<QByteArray> getBackgroundPalettes(QByteArray universalPalette);

    QByteArray getLayerOrder();
    bool wipeLayerOrderCache();

    LevelData* getFirstDataByMagic(uint32_t magicNumber, bool silentFail = false);
    std::string filename;
    bool deleteSpriteByUUID(uint32_t uuid);
    LevelObject* addSpriteData(LevelObject lo, bool newUuid = true);
private:    
    std::vector<LevelData*> subData;
    QByteArray layerOrderCache;
    uint32_t paletteRamIndex;
};