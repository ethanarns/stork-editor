#include "FsPacker.h"

#include "utils.h"

#include <vector>

std::vector<uint8_t> FsPacker::packInstruction(uint32_t instructionHex, std::vector<uint8_t> internalData, bool lz77compress) {
    std::vector<uint8_t> result;

    auto magicVec = YUtils::uint32toVec(instructionHex);

    YUtils::appendVector(result,magicVec);

    return result;
}