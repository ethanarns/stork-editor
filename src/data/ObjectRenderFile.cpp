#include "ObjectRenderFile.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"

ObjectRenderArchive::ObjectRenderArchive() {
    YUtils::printDebug("Initializing empty ObjectRenderArchive",DebugType::WARNING);
}

ObjectRenderArchive::ObjectRenderArchive(std::vector<uint8_t> obarVector) {
    if (obarVector.size() < 1) {
        YUtils::popupAlert("Empty OBAR file");
        return;
    }
    uint32_t obarIndex = 0;
    if (obarVector.at(obarIndex) == 0x10) {
        obarVector = YCompression::lz10decomp(obarVector);
    }
    auto obarDecompLength = obarVector.size();
    auto obarMagicMaybe = YUtils::getUint32FromVec(obarVector,obarIndex);
    obarIndex += 4;
    if (obarMagicMaybe != Constants::OBAR_MAGIC_NUM) {
        std::stringstream ssObarMagic;
        ssObarMagic << "File is missing OBAR magic number. Found instead: ";
        ssObarMagic << std::hex << obarMagicMaybe;
        YUtils::printDebug(ssObarMagic.str(),DebugType::ERROR);
        YUtils::popupAlert(ssObarMagic.str());
        return;
    }
    uint32_t obarInternalLength = YUtils::getUint32FromVec(obarVector,obarIndex);
    Q_UNUSED(obarInternalLength);
    obarIndex += 4;
    uint32_t globalIndex = 0;
    while (obarIndex < obarDecompLength) {
        auto headerCheck = YUtils::getUint32FromVec(obarVector,obarIndex);
        obarIndex += 4;
        auto innerLength = YUtils::getUint32FromVec(obarVector,obarIndex);
        if (innerLength <= 0x4) {
            YUtils::printDebug("Very short innerLength",DebugType::ERROR);
            return;
        }
        obarIndex += 4;
        auto innerSection = YUtils::subVector(obarVector,obarIndex,obarIndex + innerLength);
        if (headerCheck == Constants::OBJB_MAGIC_NUM) {
            uint32_t endPos = obarIndex + innerLength;
            auto objb = new ObjectTileData(obarVector, obarIndex, endPos);
            // Metadata
            objb->_obarAddress = obarIndex;
            // Push it
            this->objectTileDataVector.push_back(objb);
            this->objectTileDataMap[globalIndex] = objb;
            globalIndex++;
            obarIndex = endPos;
        } else if (headerCheck == Constants::PLTB_MAGIC_NUM || headerCheck == Constants::PALB_MAGIC_NUM) {
            auto pltbSector = YUtils::subVector(obarVector,obarIndex,obarIndex+innerLength);
            if (pltbSector.size() % Constants::PALETTE_SIZE != 0) {
                YUtils::printDebug("Uneven PLTB size",DebugType::ERROR);
            }
            uint paletteSetCount = pltbSector.size() / Constants::PALETTE_SIZE;
            // Make it
            auto objPltb = new ObjPltb();
            for (uint i = 0; i < paletteSetCount; i++) {
                QByteArray curPalette;
                curPalette.resize(Constants::PALETTE_SIZE);
                for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
                    curPalette[curPaletteIndex] = pltbSector.at(curPaletteIndex+(i*Constants::PALETTE_SIZE));
                }
                objPltb->palettes.push_back(curPalette);
            }
            // Metadata
            objPltb->_obarAddress = obarIndex;
            // Push it
            this->objectPaletteDataVector.push_back(objPltb);
            this->objectPaletteDataMap[globalIndex] = objPltb;
            globalIndex++;
            obarIndex += innerLength;
        } else if (headerCheck == Constants::OBJZ_MAGIC_NUM) {
            auto sectionCompressed = YUtils::subVector(obarVector,obarIndex,obarIndex + innerLength);
            auto decomp = YCompression::lz10decomp(sectionCompressed);
            auto objz = new ObjectTileData(decomp);
            // Metadata
            objz->_obarAddress = obarIndex;
            // Push it
            this->objectTileDataVector.push_back(objz);
            this->objectTileDataMap[globalIndex] = objz;
            globalIndex++;
            obarIndex += innerLength;
        } else {
            std::stringstream ssUnknownHeader;
            ssUnknownHeader << "Unknown header when loading OBAR file: ";
            ssUnknownHeader << std::hex << headerCheck;
            YUtils::printDebug(ssUnknownHeader.str(),DebugType::ERROR);
            globalIndex++;
            obarIndex += innerLength;
        }
    }
}

ObjectTileData::ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t end) {
    // Just take the entire chunk, it's not smart to try and calc it a different way
    this->byteData = YUtils::subVector(obarVector,obarIndex,end);
}

ObjectTileData::ObjectTileData(std::vector<uint8_t> decompVector) {
    this->byteData = decompVector;
}

ObjbFrame ObjectTileData::getFrameAt(uint32_t frameIndex) {
    // Each frame is 4 bytes
    uint32_t frameAddress = frameIndex * 4;
    ObjbFrame frame;
    frame.buildOffset = YUtils::getUint16FromVec(this->byteData,frameAddress+0);
    frame.holdTime = this->byteData.at(frameAddress+2);
    frame.frameJump = static_cast<int8_t>(this->byteData.at(frameAddress+3));
    frame._binOffset = frameAddress;
    if (frame.buildOffset == 0xffff) {
        YUtils::printDebug("FrameBuild offset was 0xffff",DebugType::VERBOSE);
        return frame;
    }
    uint32_t buildFrameLocation = frame.buildOffset + frame._binOffset;
    // This is a loop
    bool continueBuildFrameLoop = true;
    while (continueBuildFrameLoop) {
        ObjFrameBuild* frameBuild = new ObjFrameBuild();
        frameBuild->tileOffset = YUtils::getUint16FromVec(this->byteData,buildFrameLocation);
        buildFrameLocation += 2;
        frameBuild->xOffset = YUtils::getSint16FromVec(this->byteData,buildFrameLocation);
        buildFrameLocation += 2;
        frameBuild->yOffset = YUtils::getSint16FromVec(this->byteData,buildFrameLocation);
        buildFrameLocation += 2;
        frameBuild->flags = YUtils::getUint16FromVec(this->byteData,buildFrameLocation);
        buildFrameLocation += 2;
        frame.buildFrames.push_back(frameBuild);
        bool flagIndex7 = frameBuild->flags & 0b10000000;
        if (flagIndex7 == 0) {
            //YUtils::printDebug("MORE");
        } else {
            // Reached end of list, since it was 1
            continueBuildFrameLoop = false;
        }
    }
    return frame;
}

std::vector<QByteArray> ObjectTileData::getChartiles(uint32_t baseDataIndex, uint32_t countOfTiles, BgColorMode colMode) {
    std::vector<QByteArray> chartiles;
    int tileBytesSize = Constants::CHARTILE_DATA_SIZE;
    if (colMode == BgColorMode::MODE_256) {
        tileBytesSize = 64; // Since each one is a single byte, instead of split into 2, 8x8 = 64
    }
    // For each tile...
    for (uint currentTileIndex = 0; currentTileIndex < countOfTiles; currentTileIndex++) {
        uint32_t curIndex = baseDataIndex + (currentTileIndex * tileBytesSize);
        // Get a segment that is 
        auto curSection = YUtils::subVector(this->byteData,curIndex,curIndex+tileBytesSize);
        if (colMode == BgColorMode::MODE_16 || colMode == BgColorMode::MODE_UNKNOWN) {
            // This splits things up
            chartiles.push_back(YUtils::tileVectorToQByteArray(curSection));
        } else {
            QByteArray color256array;
            for (int index256 = 0; index256 < 64; index256++) {
                // Don't split bytes for the 8x8
                color256array.push_back(curSection.at(index256));
            }
            chartiles.push_back(color256array);
        }
    }
    return chartiles;
}

std::vector<QByteArray> ObjectTileData::getChartilesCompressed(uint32_t baseDataIndex, uint32_t countOfTiles, BgColorMode colMode) {
    std::vector<QByteArray> resultTiles;
    int tileBytesSize = Constants::CHARTILE_DATA_SIZE;
    if (colMode == BgColorMode::MODE_256) {
        tileBytesSize = 64; // Since each one is a single byte, instead of split into 2, 8x8 = 64
    }
    // Get the decompressed data
    uint32_t totalLength = tileBytesSize * countOfTiles;
    auto compressedVector = YUtils::subVector(this->byteData,baseDataIndex,baseDataIndex+totalLength);
    if (compressedVector.at(0) != 0x10) {
        YUtils::printDebug("First tile was not 0x10 in getChartilesCompressed",DebugType::WARNING);
        resultTiles.resize(tileBytesSize);
        return resultTiles;
    }
    auto decompressedVector = YCompression::lz10decomp(compressedVector);
    if (decompressedVector.size() == 0) {
        YUtils::printDebug("Failed to decompress vector",DebugType::ERROR);
        resultTiles.resize(tileBytesSize);
        return resultTiles;
    }
    // For each tile...
    for (uint currentTileIndex = 0; currentTileIndex < countOfTiles; currentTileIndex++) {
        // Get a segment of the uncompressed data
        uint32_t start = currentTileIndex*tileBytesSize;
        auto curSection = YUtils::subVector(decompressedVector,start,start+tileBytesSize);
        if (colMode == BgColorMode::MODE_16 || colMode == BgColorMode::MODE_UNKNOWN) {
            // This splits things up
            resultTiles.push_back(YUtils::tileVectorToQByteArray(curSection));
        } else if (colMode == BgColorMode::MODE_256) {
            QByteArray color256array;
            for (int index256 = 0; index256 < 64; index256++) {
                // Don't split bytes for the 8x8
                color256array.push_back(curSection.at(index256));
            }
            resultTiles.push_back(color256array);
        } else {
            YUtils::printDebug("Unknown color mode in getChartilesCompressed",DebugType::ERROR);
        }
    }
    return resultTiles;
}
