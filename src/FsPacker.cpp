#include "FsPacker.h"

#include "utils.h"
#include "compression.h"

#include <vector>

std::vector<uint8_t> FsPacker::packInstruction(uint32_t instructionHex, std::vector<uint8_t> internalData, bool lz77compress) {
    // Start with the magic hex
    std::vector<uint8_t> result = YUtils::uint32toVec(instructionHex);

    if (lz77compress) {
        // Without the &, internalData is COPIED into this function
        internalData = YCompression::lzssVectorRecomp(internalData);
    }

    if (lz77compress) { // Should 4 byte padding apply to ALL?
        while (internalData.size() % 4 != 0) {
            internalData.push_back(0x00);
        }
    }

    auto dataLength = YUtils::uint32toVec(internalData.size());

    YUtils::appendVector(result,dataLength);

    YUtils::appendVector(result,internalData);

    return result;
}