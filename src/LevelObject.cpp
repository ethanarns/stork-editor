#include "LevelObject.h"
#include "utils.h"

#include <vector>
#include <iostream>

#include <QByteArray>

using namespace std;

std::vector<ObjectDrawInstruction> LevelObject::getInstructionsFromObjsetRecord(std::vector<uint8_t> dataVector) {
    std::vector<ObjectDrawInstruction> result;

    uint32_t subLength = dataVector.size();
    uint32_t subIndex = 0x00;

    // First record set (good)
    bool endLoop1 = false;
    while (endLoop1 == false) {
        // First AND second byte
        uint16_t posRecordOffset = YUtils::getUint16FromVec(dataVector,subIndex+0);
        if (posRecordOffset == 0) {
            endLoop1 = true;
            break;
        }
        // 3rd byte
        uint8_t animationHoldTime = dataVector.at(subIndex+2);
        // The first number is an offset to the position data
        // It is offset FROM the current record
        uint16_t addrOfPositionRecord = subIndex + posRecordOffset;
        uint16_t frameIndex = YUtils::getUint16FromVec(dataVector,addrOfPositionRecord);
        int16_t xOffset = YUtils::getInt16FromVec(dataVector, addrOfPositionRecord + 2); // Doesn't print right, but the math works
        int16_t yOffset = YUtils::getInt16FromVec(dataVector, addrOfPositionRecord + 4); // Doesn't print right, but the math works
        uint32_t tileStart = frameIndex << 4;
        uint32_t subEnd = tileStart + Constants::CHARTILE_DATA_SIZE;
        if (tileStart+Constants::CHARTILE_DATA_SIZE > subLength) {
            cerr << "[WARN] Tried to get too big a chunk! Wanted " << hex << subEnd;
            cerr << ", only had " << hex << subLength << endl;
            //YUtils::printVector(dataVector);
            return result;
        }
        auto curTiles = YUtils::subVector(dataVector,tileStart,subEnd);

        ObjectDrawInstruction curInstruction;
        curInstruction.offsetX = xOffset;
        curInstruction.offsetY = yOffset;
        curInstruction.animationHoldTime = animationHoldTime;
        curInstruction.pixelVector = YUtils::tileVectorToQByteArray(curTiles);
        result.push_back(curInstruction);

        subIndex += 4;
    }

    return result;
}