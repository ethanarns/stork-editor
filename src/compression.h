#ifndef YIDSCOMPRESSION_H
#define YIDSCOMPRESSION_H

#include <string>
#include <filesystem>
#include <vector>

using namespace std;

class YCompression {
public:
    static bool blzDecompress(std::string filepath, bool verbose = false);
    static bool lzssDecomp(std::string filepath, bool verbose = false);
    static bool lzssRecomp(std::string filepath, bool verbose = false);
    static std::vector<uint8_t> lzssVectorDecomp(std::vector<uint8_t>& inputVec, bool verbose = false);
    static std::vector<uint8_t> lzssVectorRecomp(std::vector<uint8_t>& uncompressedVec, bool verbose = false);
    static void unpackRom(std::string romFileName);
    static void repackRom(std::string outputFileName);
};

#endif