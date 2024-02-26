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
            YUtils::printDebug("Loading OBJB");
            uint32_t endPos = obarIndex + innerLength;
            auto objb = new ObjectTileData(obarVector, obarIndex, endPos);
            this->objectTileDataVector.push_back(objb);
            if (obarIndex != endPos) {
                std::stringstream ssLengthObjb;
                ssLengthObjb << "obarIndex != endPos: 0x" << std::hex;
                ssLengthObjb << obarIndex << " != 0x" << std::hex << endPos;
                YUtils::printDebug(ssLengthObjb.str(),DebugType::WARNING);
                obarIndex = endPos;
            }
        } else if (headerCheck == Constants::PLTB_MAGIC_NUM) {
            //YUtils::printDebug("Unhandled PLTB record");
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
    std::cout << "Loop done" << std::endl;
}

ObjectTileData::ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t end) {
    // Skipped the header and length already
    // Should start with frames
    bool continueFrames = true;
    uint32_t startIndex = obarIndex + 0;
    uint32_t highestOffset = 0;
    while (continueFrames) {
        auto frameIndexLoc = obarIndex + 0;
        auto buildOffset = YUtils::getUint16FromVec(obarVector,obarIndex);
        if (buildOffset > highestOffset) {
            highestOffset = buildOffset;
        }
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
        this->frames.push_back(frame);
    }
    uint32_t indexOfFirstBuildFrame = obarIndex;
    // std::cout << "indedOfFirstBuildFrame: 0x" << std::hex << indexOfFirstBuildFrame << std::endl;
    // std::cout << "lowestOffset: 0x" << std::hex << lowestOffset << std::endl;
    // std::cout << "highestOffset: 0x" << std::hex << highestOffset << std::endl;
    uint32_t firstNotBuildFrameIndex = indexOfFirstBuildFrame + highestOffset;
    uint32_t lowestOffset = 0xffff;
    while (obarIndex < firstNotBuildFrameIndex) {
        auto frameBuildIndexLoc = obarIndex + 0;
        auto tileOffset = YUtils::getUint16FromVec(obarVector,obarIndex);
        if (tileOffset < lowestOffset) {
            lowestOffset = tileOffset;
        }
        obarIndex += 2;
        auto xOffset = YUtils::getSint16FromVec(obarVector,obarIndex);
        obarIndex += 2;
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
        // // 4 bytes worth of zeroes
        // if (tileOffset == 0x0000) {
        //     // Loop null terminated
        //     continueFrameBuilds = false;
        //     break;
        // }
        this->frameBuilds.push_back(frameBuild);
    }
    // for (int i = 0; i < this->frameBuilds.size(); i++) {
    //     std::cout << this->frameBuilds.at(i)->toString() << std::endl;
    // }
    // std::cout << "lowestOffset << 4: 0x" << std::hex << (lowestOffset << 4) << std::endl;
    // std::cout << "startIndex: 0x" << std::hex << startIndex << std::endl;
    // std::cout << "Start of chartiles: 0x" << std::hex << (startIndex + (lowestOffset << 4)) << std::endl;
    obarIndex = (startIndex + (lowestOffset << 4));
    // Now the rest of the tiles
    while (obarIndex < end) {
        auto chartileArray = YUtils::subVector(obarVector,obarIndex,obarIndex + Constants::CHARTILE_DATA_SIZE);
        auto objchar = new ObjChartile();
        objchar->_binOffset = obarIndex;
        objchar->tileVector = chartileArray;
        this->chartiles.push_back(objchar);
        obarIndex += Constants::CHARTILE_DATA_SIZE;
    }
    // if (obarIndex != end) {
    //     std::cout << "Not match index vs end" << std::endl;
    //     std::cout << std::hex << obarIndex << std::endl;
    //     std::cout << std::hex << end << std::endl;
    //     exit(EXIT_FAILURE);
    // }
}
