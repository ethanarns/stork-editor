#include "MapData.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"
#include "../Chartile.h"

#include <vector>
#include <fstream>
#include <QByteArray>

LayerData::LayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, uint32_t &globalPaletteIndex) {
    while (mpdzIndex < stop) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t tempEnd = mpdzIndex + subLength;
        if (subMagic == Constants::INFO_MAGIC_NUM) {
            auto info = new ScenInfoData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(info);
        } else if (subMagic == Constants::ANMZ_MAGIC_NUM) {
            auto anmz = new AnimatedMapData(mpdzBytes,mpdzIndex,mpdzIndex+subLength,this->getInfo()->colorMode);
            this->subScenData.push_back(anmz);
        } else if (subMagic == Constants::IMGB_MAGIC_NUM) {
            auto imgb = new ImgbLayerData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(imgb);
        } else if (subMagic == Constants::PLTB_MAGIC_NUM) {
            this->paletteStartOffset = globalPaletteIndex;
            auto pltb = new LayerPaletteData(mpdzBytes,mpdzIndex,mpdzIndex+subLength,globalPaletteIndex);
            this->subScenData.push_back(pltb);
        } else if (subMagic == Constants::COLZ_MAGIC_NUM) {
            auto colz = new MapCollisionData(mpdzBytes,mpdzIndex,mpdzIndex+subLength);
            this->subScenData.push_back(colz);
        } else if (subMagic == Constants::MPBZ_MAGIC_NUM) {
            auto mpbz = new MapTilesData(mpdzBytes,mpdzIndex,mpdzIndex+subLength, this);
            this->subScenData.push_back(mpbz);
        } else if (subMagic == Constants::IMBZ_MAGIC_NUM) {
            auto imbz = new ImbzLayerData(mpdzBytes,mpdzIndex,mpdzIndex+subLength,this->getInfo()->colorMode);
            this->subScenData.push_back(imbz);
        } else {
            std::stringstream unknownMagic;
            unknownMagic << "Unknown magic number in LayerData: 0x" << std::hex << subMagic;
            YUtils::printDebug(unknownMagic.str(),DebugType::ERROR);
            mpdzIndex += subLength;
        }
        if (mpdzIndex != tempEnd) {
            std::stringstream ssEndNotMatch;
            ssEndNotMatch << "Mismatch in end index. Current Index: " << std::hex;
            ssEndNotMatch << mpdzIndex << ", Temp End: " << tempEnd;
            ssEndNotMatch << ", Magic Number: " << std::hex << subMagic;
            YUtils::printDebug(ssEndNotMatch.str(),DebugType::ERROR);
        }
    }
}

LayerData::~LayerData() {
    for (auto it = this->subScenData.begin(); it != this->subScenData.end(); ) {
        delete (*it);
        it = this->subScenData.erase(it);
    }
    this->subScenData.shrink_to_fit();
    this->cachedImbzTileData.clear();
    this->cachedImbzTileData.shrink_to_fit();
    this->cachedVramTiles.clear();
}

ScenInfoData::ScenInfoData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    this->layerWidth = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 2;
    this->layerHeight = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 2;
    this->bgYoffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    this->xScrollOffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    this->yScrollOffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    this->whichBackground = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->layerOrder = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->unkThird = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    this->baseBlockMaybe = mpdzBytes.at(mpdzIndex);
    mpdzIndex++;
    auto colorMode = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    this->colorMode = colorMode == 0 ? BgColorMode::MODE_16 : BgColorMode::MODE_256;
    mpdzIndex += 4;
    if (mpdzIndex == stop) {
        //YUtils::printDebug("No IMBZ string",DebugType::VERBOSE);
        return;
    }
    auto charFileNoExt = YUtils::getNullTermTextFromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += charFileNoExt.size() + 1;
    this->imbzFilename = charFileNoExt;
    while (mpdzIndex % 4 != 0) {
        mpdzIndex++;
    }
}

ScenInfoData::ScenInfoData() {
    YUtils::printDebug("Creating empty ScenInfoData",DebugType::VERBOSE);
}

LayerPaletteData::LayerPaletteData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, uint32_t &globalPaletteIndex) {
    while (mpdzIndex < stop) {
        auto currentLoadingPalette = new QByteArray();
        currentLoadingPalette->resize(Constants::PALETTE_SIZE);
        for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
            (*currentLoadingPalette)[curPaletteIndex] = mpdzBytes.at(mpdzIndex);
            mpdzIndex++;
        }
        this->palettes.push_back(currentLoadingPalette);
        Q_UNUSED(globalPaletteIndex);
        //globalPaletteIndex++;
    }
}

LayerPaletteData::~LayerPaletteData() {
    for (auto it = this->palettes.begin(); it != this->palettes.end(); ) {
        delete (*it);
        it = this->palettes.erase(it);
    }
    this->palettes.shrink_to_fit();
}

// MPBZ
MapTilesData::MapTilesData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, LayerData* parent) {
    auto info = parent->getInfo();
    if (info == nullptr) {
        YUtils::printDebug("INFO not loaded before MapTiles/MPBZ",DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    this->tileRenderData.reserve(180'000);
    auto compressed = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    auto mpbzData = YCompression::lz10decomp(compressed);
    uint32_t mIndex = 0;
    if (YUtils::getUint16FromVec(mpbzData,mIndex) == 0xffff) {
        mIndex += 2;
        // Offset is active
        this->tileOffset = YUtils::getUint16FromVec(mpbzData,mIndex);
        mIndex += 2;
        this->bottomTrim = YUtils::getUint16FromVec(mpbzData,mIndex);
        mIndex += 2;
        uint32_t offset = this->tileOffset * info->layerWidth;
        for (uint32_t offsetWriteIndex = 0; offsetWriteIndex < offset; offsetWriteIndex++) {
            this->tileRenderData.push_back(0x0000);
        } 
        //mIndex += 3; // 0x0201c714 Huh? This doesn't make sense. Goes ahead 4, but changing to -1 breaks
    } else {
        this->tileOffset = 0;
        this->bottomTrim = 0;
    }
    // Loop
    auto end = mpbzData.size();
    while (mIndex < end) {
        uint16_t curShort = YUtils::getUint16FromVec(mpbzData,mIndex);
        mIndex += 2;
        if (info->colorMode == BgColorMode::MODE_16) {
            curShort += 0x1000; // 0201c730
        }
        this->tileRenderData.push_back(curShort);
    }
    mpdzIndex += compressed.size();
}

MapTilesData::~MapTilesData() {
    this->tileRenderData.clear();
    this->tileRenderData.shrink_to_fit();
}

AnimatedMapData::AnimatedMapData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, BgColorMode &colorMode) {
    auto compressed = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    auto anmzData = YCompression::lz10decomp(compressed);
    const uint32_t anmzSize = anmzData.size();
    uint32_t anmzIndex = 0;
    this->frameCount = anmzData.at(anmzIndex);
    anmzIndex++;
    this->unknown1 = anmzData.at(anmzIndex);
    anmzIndex++;
    this->unknown2 = YUtils::getUint16FromVec(anmzData,anmzIndex);
    anmzIndex += 2;
    this->vramOffset = YUtils::getUint16FromVec(anmzData,anmzIndex);
    anmzIndex += 2;
    this->unknown3 = anmzData.at(anmzIndex);
    anmzIndex++;
    this->unknown4 = anmzData.at(anmzIndex);
    anmzIndex++;
    this->frameTimes.clear();
    for (uint32_t i = 0; i < this->frameCount; i++) {
        this->frameTimes.push_back(anmzData.at(anmzIndex));
        anmzIndex++;
    }
    // 4 byte padding
    while (anmzIndex % 4 != 0) {
        anmzIndex++;
    }
    uint32_t currentTileIndex = 0;
    while (anmzIndex < anmzSize) {
        Chartile curTile;
        curTile.index = currentTileIndex;
        curTile.tiles.resize(64);
        if (colorMode == BgColorMode::MODE_16) {
            curTile.bgColMode = BgColorMode::MODE_16;
            for (
                int currentTileBuildIndex = 0;
                currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE;
                currentTileBuildIndex++
            ) {
                uint8_t curByte = anmzData.at(anmzIndex);
                anmzIndex++;
                uint8_t highBit = curByte >> 4;
                uint8_t lowBit = curByte % 0x10;
                int innerPosition = currentTileBuildIndex*2;
                curTile.tiles[innerPosition+1] = highBit;
                curTile.tiles[innerPosition+0] = lowBit;
            }
        } else {
            curTile.bgColMode = BgColorMode::MODE_256;
            // Instead of being split, each pixel is a full byte
            // Meaning 64 places
            for (
                int currentTileBuildIndex = 0;
                currentTileBuildIndex < 64;
                currentTileBuildIndex++
            ) {
                uint8_t curByte = anmzData.at(anmzIndex);
                anmzIndex++;
                curTile.tiles[currentTileBuildIndex] = curByte;
            }
        }
        this->chartiles.push_back(curTile);
    }
    // Index was separate
    mpdzIndex += compressed.size();
}

AnimatedMapData::~AnimatedMapData() {
    this->frameTimes.clear();
    this->frameTimes.shrink_to_fit();
    this->chartiles.clear();
    this->chartiles.shrink_to_fit();
}

// COLZ
MapCollisionData::MapCollisionData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    auto compressed = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    this->colData.reserve(79'000);
    this->colData = YCompression::lz10decomp(compressed);
    mpdzIndex += compressed.size();
}

MapCollisionData::~MapCollisionData() {
    this->colData.clear();
    this->colData.shrink_to_fit();
}

ImgbLayerData::ImgbLayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    uint32_t debugIndex = 0;
    while (mpdzIndex < stop) {
        Chartile curTile;
        curTile.bgColMode = BgColorMode::MODE_16;
        curTile.index = debugIndex;
        debugIndex++;
        curTile.tiles.resize(64);
        for (
            int currentTileBuildIndex = 0;
            currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE;
            currentTileBuildIndex++
        ) {
            uint8_t curByte = mpdzBytes.at(mpdzIndex);
            mpdzIndex++;
            uint8_t highBit = curByte >> 4;
            uint8_t lowBit = curByte % 0x10;
            int innerPosition = currentTileBuildIndex*2;
            curTile.tiles[innerPosition+1] = highBit;
            curTile.tiles[innerPosition+0] = lowBit;
        }
        this->chartiles.push_back(curTile);
    }
}

ImgbLayerData::~ImgbLayerData() {
    this->chartiles.clear();
    this->chartiles.shrink_to_fit();
}

// GRAD
LevelGradientData::LevelGradientData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    // It is GINF then GCOL every single time
    while (mpdzIndex < stop) {
        uint32_t subMagic = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t subLength = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 4;
        uint32_t tempEnd = mpdzIndex + subLength;
        if (subMagic == Constants::GINF_MAGIC_NUM) {
            this->unknown1 = mpdzBytes.at(mpdzIndex++);
            this->unknown2 = mpdzBytes.at(mpdzIndex++);
            this->unknown3 = mpdzBytes.at(mpdzIndex++);
            this->unknown4 = mpdzBytes.at(mpdzIndex++);
            this->unknown5 = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 2;
            this->unknown6 = mpdzBytes.at(mpdzIndex++);
            this->unknown7 = mpdzBytes.at(mpdzIndex++);
            this->yOffset = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 4;
        } else if (subMagic == Constants::GCOL_MAGIC_NUM) {
            uint32_t stop = mpdzIndex+subLength;
            while (mpdzIndex < stop) {
                auto curColor = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
                mpdzIndex += 2;
                this->colors.push_back(curColor);
            }
        } else {
            std::stringstream unknownMagic;
            unknownMagic << "Unknown magic number in LevelGradientData: 0x" << std::hex << subMagic;
            YUtils::printDebug(unknownMagic.str(),DebugType::ERROR);
            mpdzIndex += subLength;
        }
        if (mpdzIndex != tempEnd) {
            std::stringstream ssEndNotMatch;
            ssEndNotMatch << "Mismatch in end index. Current Index: " << std::hex;
            ssEndNotMatch << mpdzIndex << ", Temp End: " << tempEnd;
            ssEndNotMatch << ", Magic Number in GRAD: " << std::hex << subMagic;
            YUtils::printDebug(ssEndNotMatch.str(),DebugType::ERROR);
        }
    }
}

// SETD
LevelObjectData::LevelObjectData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    while (mpdzIndex < stop) {
        auto lo = new LevelObject();
        lo->uuid = this->uuidIndex++;
        lo->objectId = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        lo->settingsLength = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        lo->xPosition = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        lo->yPosition = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        for (uint16_t i = 0; i < lo->settingsLength; i++) {
            auto curSetByte = mpdzBytes.at(mpdzIndex);
            mpdzIndex++;
            lo->settings.push_back(curSetByte);
        }
        // Done!
        this->levelObjects.push_back(lo);
    }
    if (mpdzIndex != stop) {
        YUtils::printDebug("Mismatch in SETD end address",DebugType::ERROR);
    }
}

LevelObjectData::~LevelObjectData() {
    for (auto it = this->levelObjects.begin(); it != this->levelObjects.end(); ) {
        delete (*it);
        it = this->levelObjects.erase(it);
    }
    this->levelObjects.shrink_to_fit();
}

uint32_t triggerBoxId = 1;
TriggerBoxData::TriggerBoxData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    while (mpdzIndex < stop) {
        auto triggerBox = new TriggerBox();
        triggerBox->leftX = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        triggerBox->topY = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        triggerBox->rightX = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        triggerBox->bottomY = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
        mpdzIndex += 2;
        triggerBox->uuid = triggerBoxId++;
        this->triggers.push_back(triggerBox);
        if (triggerBoxId > 250) { // Can't go above 0xff
            triggerBoxId = 1; // Loop
        }
    }
    if (mpdzIndex != stop) {
        YUtils::printDebug("MPDZ index did not match stop value in TriggerBoxData",DebugType::ERROR);
    }
}

PathData::PathData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    //std::cout << "PathData()" << std::endl;
    // Not compressed
    uint32_t totalPaths = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
    mpdzIndex += 4;
    if (this->paths.size() != 0) {
        YUtils::printDebug("Paths size not zero at start!",DebugType::ERROR);
        mpdzIndex = stop;
        return;
    }
    for (uint i = 0; i < totalPaths; i++) {
        std::vector<PathSection*> currentPath;
        bool continuePath = true;
        while (continuePath) {
            auto angle = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 2;
            auto distance = YUtils::getUint16FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 2;
            auto fineX = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 4;
            auto fineY = YUtils::getUint32FromVec(mpdzBytes,mpdzIndex);
            mpdzIndex += 4;
            PathSection* p = new PathSection();
            p->angle = angle;
            p->distance = distance;
            p->xFine = fineX;
            p->yFine = fineY;
            currentPath.push_back(p);
            if (distance == 0x0000) {
                continuePath = false;
            }
        }
        this->paths.push_back(currentPath);
    }
    if (this->paths.size() != totalPaths) {
        YUtils::printDebug("Path result size and totalPaths don't match",DebugType::ERROR);
    }
    if (mpdzIndex != stop) {
        YUtils::printDebug("mpdzIndex does not equal stop in PathData()");
    }
    //std::cout << "Got " << std::hex << this->paths.size() << " Path vectors in PATH";
}

AlphaData::AlphaData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop) {
    Q_UNUSED(stop);
    YUtils::printDebug("ALPH data not yet fully understood, but will (probably) recompile correctly",DebugType::WARNING);
    this->byte1 = mpdzBytes.at(mpdzIndex++);
    this->byte2 = mpdzBytes.at(mpdzIndex++);
    this->byte3 = mpdzBytes.at(mpdzIndex++);
    this->byte4 = mpdzBytes.at(mpdzIndex++);
    if (mpdzIndex != stop) {
        YUtils::printDebug("ALPH decompilation length mismatch",DebugType::ERROR);
    }
}

ImbzLayerData::ImbzLayerData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex, uint32_t stop, BgColorMode bgColMode) {
    //YUtils::printDebug("ImbzLayerData");
    auto sub = YUtils::subVector(mpdzBytes,mpdzIndex,stop);
    auto uncompressedImbz = YCompression::lz10decomp(sub);

    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles
    int imbzIndex = 0; // Goes up by 0x20/32 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    if (imbzLength < 1) {
        YUtils::printDebug("IMBZ data size is 0!",DebugType::ERROR);
        return;
    }

        // Do it 0x20 by 0x20 (32)
    while (imbzIndex < imbzLength) { // Kill when equal to length, meaning it's outside
        Chartile curTile;
        curTile.index = currentTileIndex;
        curTile.tiles.resize(64);
        if (bgColMode == BgColorMode::MODE_16) {
            curTile.bgColMode = BgColorMode::MODE_16;
            // Go up by 2 since you split the bytes
            for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
                uint8_t curByte = uncompressedImbz.at(imbzIndex + currentTileBuildIndex);
                uint8_t highBit = curByte >> 4;
                uint8_t lowBit = curByte % 0x10;
                int innerPosition = currentTileBuildIndex*2;
                curTile.tiles[innerPosition+1] = highBit;
                curTile.tiles[innerPosition+0] = lowBit;
            }
        } else {
            curTile.bgColMode = BgColorMode::MODE_256;
            for (int currentTileIndex = 0; currentTileIndex < 64; currentTileIndex++) {
                uint8_t curByte = uncompressedImbz.at(imbzIndex + currentTileIndex);
                curTile.tiles[currentTileIndex] = curByte;
            }
        }
        this->chartiles.push_back(curTile);
        
        if (bgColMode == BgColorMode::MODE_16) {
            // Skip ahead by 0x20/32
            imbzIndex += Constants::CHARTILE_DATA_SIZE;
        } else {
            imbzIndex += 64;
        }
        currentTileIndex++;
    }

    // Jump to the end
    mpdzIndex = stop;
}

ImbzLayerData::~ImbzLayerData() {
    this->chartiles.clear();
    this->chartiles.shrink_to_fit();
}
