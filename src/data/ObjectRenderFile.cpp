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
        obarIndex += 4;
        auto innerSection = YUtils::subVector(obarVector,obarIndex,obarIndex + innerLength);
        if (headerCheck == Constants::OBJB_MAGIC_NUM) {
            YUtils::printDebug("Loading OBJB");
            uint32_t endPos = obarIndex + innerLength;
            auto objb = new ObjectTileData(obarVector, obarIndex, innerLength);
            this->objectTileDataVector.push_back(objb);
            std::cout << "pushed back" << std::endl;
            obarIndex = endPos;
        } else if (headerCheck == Constants::PLTB_MAGIC_NUM) {
            YUtils::printDebug("Unhandled PLTB record");
            obarIndex += innerLength;
        } else if (headerCheck == Constants::OBJZ_MAGIC_NUM) {
            YUtils::printDebug("Unhandled OBJZ record");
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

ObjectTileData::ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t length) {
    // Skipped the header and length already
    // Should start with frames
    bool continueFrames = true;
    while (continueFrames) {
        auto frameIndexLoc = obarIndex + 0;
        auto buildOffset = YUtils::getUint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
        auto holdTime = obarVector.at(obarIndex);
        obarIndex++;
        auto frameJump = static_cast<int8_t>(obarVector.at(obarIndex));
        obarIndex++;
        if (buildOffset == 0x0000 && holdTime == 0x00 && frameJump == 00) {
            // Loop null terminated
            continueFrames = false;
            break;
        }
        auto frame = new ObjbFrame();
        frame->buildOffset = buildOffset;
        frame->holdTime = holdTime;
        frame->frameJump = frameJump;
        frame->_binOffset = frameIndexLoc;
        this->frames.push_back(frame);
    }
    // Onto the next
    bool continueFrameBuilds = true;
    while (continueFrameBuilds) {
        auto frameBuildIndexLoc = obarIndex + 0;
        auto tileOffset = YUtils::getUint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
        auto xOffset = YUtils::getSint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
        // 4 bytes worth of zeroes
        if (tileOffset == 0x0000 && xOffset == 0x0000) {
            // Loop null terminated
            continueFrameBuilds = false;
            break;
        }
        auto yOffset = YUtils::getSint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
        auto flags = YUtils::getUint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
        auto frameBuild = new ObjFrameBuild();
        frameBuild->tileOffset = tileOffset;
        frameBuild->xOffset = xOffset;
        frameBuild->yOffset = yOffset;
        frameBuild->flags = flags;
        frameBuild->_binOffset = frameBuildIndexLoc;
        this->frameBuilds.push_back(frameBuild);
    }
    // Now the rest of the tiles
    std::cout << "Remaining tile time" << std::endl;
    for (int i = 0; i < this->frameBuilds.size(); i++) {
        auto chartileArray = YUtils::subVector(obarVector,obarIndex,obarIndex + Constants::CHARTILE_DATA_SIZE);
        auto objchar = new ObjChartile();
        objchar->_binOffset = obarIndex;
        objchar->tileVector = chartileArray;
        this->chartiles.push_back(objchar);
        obarIndex += Constants::CHARTILE_DATA_SIZE;
    }
    std::cout << "Done!" << std::endl;
}
