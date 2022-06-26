#ifndef YUTILS_H
#define YUTILS_H

#include "Chartile.h"

#include <string>
#include <filesystem>
#include <vector>
#include <fstream>

#include <QtCore>
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
    static ChartilePreRenderData getCharPreRender(uint16_t tileAttr);

    static std::vector<uint8_t> subVector(std::vector<uint8_t> inVec, uint32_t startOffset, uint32_t endOffset);
    static void joinVectors(std::vector<uint8_t> &firstVec, std::vector<uint8_t> &secondVec, std::vector<uint8_t> &resultVec);
    static void appendVector(std::vector<uint8_t> &baseVec, std::vector<uint8_t> &appendedVec);
    static std::vector<uint8_t> createInstructionVector(std::vector<uint8_t> &instructionVector, std::vector<uint8_t> &data);
    static void printVector(std::vector<uint8_t> &vectorToPrint, int newlineBreak = 0);
};

#endif