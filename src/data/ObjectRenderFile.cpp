#include "ObjectRenderFile.h"

#include "../utils.h"
#include "../compression.h"
#include "../constants.h"

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
    obarIndex += 4;
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
            //YUtils::printDebug("Loading OBJB");
            uint32_t endPos = obarIndex + innerLength;
            auto objb = new ObjectTileData(obarVector, obarIndex, endPos);
            this->objectTileDataVector.push_back(objb);
            obarIndex = endPos;
        } else if (headerCheck == Constants::PLTB_MAGIC_NUM) {
            //YUtils::printDebug("Unhandled PLTB record");
            obarIndex += innerLength;
        } else if (headerCheck == Constants::OBJZ_MAGIC_NUM) {
            auto sectionCompressed = YUtils::subVector(obarVector,obarIndex,obarIndex + innerLength);
            auto decomp = YCompression::lz10decomp(sectionCompressed);
            auto objz = new ObjectTileData(decomp);
            this->objectTileDataVector.push_back(objz);
            obarIndex += innerLength;
        } else {
            std::stringstream ssUnknownHeader;
            ssUnknownHeader << "Unknown header when loading OBAR file: ";
            ssUnknownHeader << std::hex << headerCheck;
            YUtils::printDebug(ssUnknownHeader.str(),DebugType::ERROR);
            obarIndex += innerLength;
        }
    }
}

ObjectTileData::ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t end) {
    uint32_t startIndex = obarIndex + 0;
    // Just take the entire chunk, it's not smart to try and calc it a different way
    this->byteData = YUtils::subVector(obarVector,startIndex,end);
    bool continueFrames = true;
    while (continueFrames) {
        auto frameIndexLoc = obarIndex + 0;
        auto buildOffset = YUtils::getUint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
        auto holdTime = obarVector.at(obarIndex);
        obarIndex++;
        auto frameJump = static_cast<int8_t>(obarVector.at(obarIndex));
        obarIndex++;
        if (buildOffset == 0x0000) {
            // Seemingly how its checked in code
            // Analysis wise, it is not padded anything above 0x4
            continueFrames = false;
            break;
        }
        auto frame = new ObjbFrame();
        frame->buildOffset = buildOffset;
        frame->holdTime = holdTime;
        frame->frameJump = frameJump;
        frame->_binOffset = frameIndexLoc;
        // Find the build frame
        uint32_t buildFrameLocation = frame->buildOffset + frame->_binOffset;
        auto tileOffset = YUtils::getUint16FromVec(obarVector,buildFrameLocation);
        auto xOffset = YUtils::getSint16FromVec(obarVector,buildFrameLocation+2);
        auto yOffset = YUtils::getSint16FromVec(obarVector,buildFrameLocation+4);
        auto flags = YUtils::getUint16FromVec(obarVector,buildFrameLocation+6);
        auto frameBuild = new ObjFrameBuild();
        frameBuild->tileOffset = tileOffset;
        frameBuild->xOffset = xOffset;
        frameBuild->yOffset = yOffset;
        frameBuild->flags = flags;
        frameBuild->_binOffset = buildFrameLocation;
        // Attach the build frame
        frame->buildFrame = frameBuild;
        this->frames.push_back(frame);
    }
}

ObjectTileData::ObjectTileData(std::vector<uint8_t> decompVector) {
    this->byteData = decompVector;
    bool continueFrames = true;
    uint32_t obarIndex = 0;
    while (continueFrames) {
        auto frameIndexLoc = obarIndex + 0;
        auto buildOffset = YUtils::getUint16FromVec(decompVector,obarIndex);
        obarIndex += 2;
        auto holdTime = decompVector.at(obarIndex);
        obarIndex++;
        auto frameJump = static_cast<int8_t>(decompVector.at(obarIndex));
        obarIndex++;
        if (buildOffset == 0x0000) {
            // Seemingly how its checked in code
            // Analysis wise, it is not padded anything above 0x4
            continueFrames = false;
            break;
        }
        auto frame = new ObjbFrame();
        frame->buildOffset = buildOffset;
        frame->holdTime = holdTime;
        frame->frameJump = frameJump;
        frame->_binOffset = frameIndexLoc;
        // Find the build frame
        uint32_t buildFrameLocation = frame->buildOffset + frame->_binOffset;
        auto tileOffset = YUtils::getUint16FromVec(decompVector,buildFrameLocation);
        auto xOffset = YUtils::getSint16FromVec(decompVector,buildFrameLocation+2);
        auto yOffset = YUtils::getSint16FromVec(decompVector,buildFrameLocation+4);
        auto flags = YUtils::getUint16FromVec(decompVector,buildFrameLocation+6);
        auto frameBuild = new ObjFrameBuild();
        frameBuild->tileOffset = tileOffset;
        frameBuild->xOffset = xOffset;
        frameBuild->yOffset = yOffset;
        frameBuild->flags = flags;
        frameBuild->_binOffset = buildFrameLocation;
        // Attach the build frame
        frame->buildFrame = frameBuild;
        this->frames.push_back(frame);
    }
}

ObjbFrame* ObjectTileData::getFrameData(uint32_t frameIndex) {
    if (frameIndex >= this->frames.size()) {
        YUtils::printDebug("frameIndex out of range",DebugType::ERROR);
        YUtils::popupAlert("frameIndex out of range");
    }
    return this->frames.at(frameIndex);
}

std::vector<QByteArray> ObjectTileData::getChartiles(uint32_t index, uint32_t count) {
    std::vector<QByteArray> chartiles;
    for (uint i = 0; i < count; i++) {
        uint32_t curIndex = index + (i * Constants::CHARTILE_DATA_SIZE);
        auto curSection = YUtils::subVector(this->byteData,curIndex,curIndex+Constants::CHARTILE_DATA_SIZE);
        chartiles.push_back(YUtils::tileVectorToQByteArray(curSection));
    }
    return chartiles;
}
