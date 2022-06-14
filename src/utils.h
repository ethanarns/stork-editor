#ifndef YUTILS_H
#define YUTILS_H

#include <string>
#include <filesystem>
#include <vector>
#include <fstream>

#include <QColor>

using namespace std;

class YUtils {
public:
    static void printSurroundingFiles(std::string path);
    static std::string getLowercase(std::string input);
    static bool writeByteVectorToFile(std::vector<uint8_t> &bytes, std::string filename);

    static size_t getStreamLength(std::ifstream& buffer);
    static uint32_t getUint32FromVec(std::vector<uint8_t> &bytes, uint32_t location);
    static std::string getNullTermTextFromVec(std::vector<uint8_t> &bytes, uint32_t location);
    static QColor getColorFromBytes(uint8_t firstByte, uint8_t secondByte);
};

#endif