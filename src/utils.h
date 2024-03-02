#ifndef YUTILS_H
#define YUTILS_H

#include "Chartile.h"
#include "constants.h"
#include "LevelObject.h"

#include <string>
#include <vector>
#include <fstream>

#include <QtCore>
#include <QColor>
#include <QByteArray>

enum DebugType {
    VERBOSE,
    WARNING,
    ERROR,
    FATAL
};

class YUtils {
public:
    static void printSurroundingFiles(std::string path);
    static std::string getLowercase(std::string input);
    static std::string getUppercase(std::string input);
    static Address conv2xAddrToFileAddr(AddressMemory x2address);
    static bool writeByteVectorToFile(std::vector<uint8_t> &bytes, std::string filename);

    static std::vector<uint8_t> getUint8VectorFromFile(std::string fileToLoad);

    static size_t getStreamLength(std::ifstream& buffer);
    static uint32_t getUint32FromVec(std::vector<uint8_t> &bytes, uint32_t location);
    static uint16_t getUint16FromVec(std::vector<uint8_t> &bytes, uint32_t location);
    static int16_t getSint16FromVec(std::vector<uint8_t> &bytes, uint32_t location);
    static std::vector<uint8_t> uint32toVec(uint32_t inputInt);
    static std::vector<uint8_t> uint16toVec(uint16_t inputInt);
    static std::string getNullTermTextFromVec(std::vector<uint8_t> &bytes, uint32_t location);
    static std::string getFixedTextFromVec(std::vector<uint8_t> &bytes, uint32_t location, uint32_t length);
    static QColor getColorFromBytes(uint8_t firstByte, uint8_t secondByte);
    static MapTileRecordData getMapTileRecordDataFromShort(uint16_t tileAttr, BgColorMode bgColorMode = BgColorMode::MODE_16);
    static void printLevelObject(LevelObject lo);
    static std::vector<uint8_t> compileObject(LevelObject lo);
    static std::vector<uint8_t> stringToVector(std::string &inputString);
    static std::string relativeToEscapedAbs(std::string relPath);
    
    static int16_t roundI16Down(int16_t unrounded, int16_t multiple);

    static std::vector<uint8_t> subVector(std::vector<uint8_t> &inVec, uint32_t startOffset, uint32_t endOffset);
    static void joinVectors(std::vector<uint8_t> &firstVec, std::vector<uint8_t> &secondVec, std::vector<uint8_t> &resultVec);
    static void appendVector(std::vector<uint8_t> &baseVec, std::vector<uint8_t> &appendedVec);
    static std::vector<uint8_t> createInstructionVector(std::vector<uint8_t> &instructionVector, std::vector<uint8_t> &data);
    static std::vector<uint8_t> createInstVecFromNum(uint32_t instCode, std::vector<uint8_t> &data);
    static void printVector(std::vector<uint8_t> &vectorToPrint, int newlineBreak = 0x10);
    static void printVector16(std::vector<uint16_t> &vectorToPrint, int newlineBreak = 0x10);
    static void writeVectorToFile(std::vector<uint8_t> &dataToWrite,std::string fileOnSystem,uint32_t addressOffset);
    static QByteArray tileVectorToQByteArray(std::vector<uint8_t> tileVector);

    static void printDebug(std::string msg, DebugType dt = DebugType::VERBOSE);
    static void printQbyte(QByteArray& qb, int newlineBreak = 0x10);
    static void popupAlert(std::string msg);
};

#endif