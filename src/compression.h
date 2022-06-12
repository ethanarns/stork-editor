#ifndef YIDSCOMPRESSION_H
#define YIDSCOMPRESSION_H

#include <string>
#include <filesystem>

using namespace std;

class YCompression {
public:
    static bool blzDecompress(std::string filepath);
    static bool lzssDecomp(std::string filepath);
};

#endif