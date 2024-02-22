#ifndef YIDSCOMPRESSION_H
#define YIDSCOMPRESSION_H

#include <string>
#include <filesystem>
#include <vector>

class YCompression {
public:
    static bool blzDecompress(std::string filepath);
    static bool lzssRecomp(std::string filepath);
    static std::vector<uint8_t> lzssVectorRecomp(std::vector<uint8_t>& uncompressedVec);
    static void unpackRom(std::string romFileName);
    static void repackRom(std::string outputFileName);

    static std::vector<uint8_t> lz10decomp(std::vector<uint8_t> data);
    static std::filesystem::path getAbsoluteRomPart(std::string dataName);
};

#endif