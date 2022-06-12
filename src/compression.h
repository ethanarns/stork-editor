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
    static std::vector<uint8_t> lzssVectorDecomp(std::vector<uint8_t>& inputVec);
};

#endif