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
            auto objb = new ObjectTileData(obarVector, obarIndex, innerLength);
            for (int i = 0; i < objb->frames.size(); i++) {
                std::cout << objb->frames.at(i)->toString() << std::endl;
            }
            for (int y = 0; y < objb->frameBuilds.size(); y++) {
                std::cout << objb->frameBuilds.at(y)->toString() << std::endl;
            }
            return;
        }
    }
}

ObjectTileData::ObjectTileData(std::vector<uint8_t> &obarVector, uint32_t &obarIndex, uint32_t length) {
    // Skipped the header and length already
    // Should start with frames
    bool continueFrames = true;
    while (continueFrames) {
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
        this->frames.push_back(frame);
    }
    // Onto the next
    bool continueFrameBuilds = true;
    while (continueFrameBuilds) {
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
        this->frameBuilds.push_back(frameBuild);
    }
}
