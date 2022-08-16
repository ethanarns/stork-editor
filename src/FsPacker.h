#ifndef FSPACKER_H
#define FSPACKER_H

#include <vector>
#include <cstdint>

class FsPacker {
public:
    static std::vector<uint8_t> packInstruction(uint32_t instructionHex, std::vector<uint8_t> internalData, bool lz77compress = false);
};

#endif