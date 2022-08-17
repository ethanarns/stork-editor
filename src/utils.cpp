#include "utils.h"
#include "constants.h"
#include "Chartile.h"

// std::cerr, std::endl, std::ios
#include <iostream>
// filesystem::current_path
#include <filesystem>
// std::tolower
#include <cctype>

#include <vector>
#include <fstream>

#include <QtCore>
#include <QColor>

using namespace std;

/**
 * @brief I am amazed this is not in STD
 * 
 * @param input String to make lowercase
 * @return std::string 
 */
std::string YUtils::getLowercase(std::string input) {
    std::stringstream out;
    for (auto elem: input) {
        out << (char)tolower(elem);
    }
    return out.str();
}

std::string YUtils::getUppercase(std::string input) {
    std::stringstream out;
    for (auto elem: input) {
        out << (char)toupper(elem);
    }
    return out.str();
}

void YUtils::printSurroundingFiles(std::string path) {
    for (const auto entry : filesystem::directory_iterator(path)) {
        std::string curFileName = entry.path().filename().string();
        cout << curFileName << endl;
    }
}

/**
 * @brief Outputs a byte vector (
 * 
 * @param bytes uint8_t byte vector address
 * @param filename output filename
 * @return true if success, false if failure
 */
bool YUtils::writeByteVectorToFile(std::vector<uint8_t> &bytes, std::string filename) {
    std::ofstream outfile(filename, ios::out | ios::binary);
    outfile.write((const char*)&bytes[0], bytes.size());
    outfile.close();
    return true;
}

uint32_t YUtils::getUint32FromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    return (uint32_t)(bytes.at(location+3) << 24) + (uint32_t)(bytes.at(location+2) << 16) + (uint32_t)(bytes.at(location+1) << 8) + (uint32_t)bytes.at(location);
}

uint16_t YUtils::getUint16FromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    return (uint16_t)(bytes.at(location+1) << 8) + (uint16_t)bytes.at(location);
}

int16_t YUtils::getInt16FromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    uint16_t first = bytes.at(location+1) << 8;
    int16_t firstS;
    // Cause it shows up as 0xFFFx
    if (first <= INT16_MAX) {
        firstS = static_cast<int16_t>(first);
    } else if (first >= INT16_MIN) {
        firstS = static_cast<int16_t>(first - INT16_MIN) + INT16_MIN;
    }
    uint16_t second = bytes.at(location);
    int16_t secondS;
    if (second <= INT16_MAX) {
        secondS = static_cast<int16_t>(second);
    } else if (second >= INT16_MIN) {
        secondS = static_cast<int16_t>(second - INT16_MIN) + INT16_MIN;
    }
    return firstS + secondS;
}

size_t YUtils::getStreamLength(std::ifstream& buffer) {
    uint32_t priorPos = buffer.tellg();
    buffer.seekg(0, ios_base::end);
    size_t result = buffer.tellg();
    buffer.seekg(priorPos, ios_base::beg);
    return result;
}

std::string YUtils::getNullTermTextFromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    uint8_t MAX_STRING_LENGTH = 0xff;
    const uint8_t NULL_TERM = 0x00;
    uint8_t killOffset = 0;
    std::string result = "";
    
    if (bytes.at(location) == NULL_TERM) {
        std::stringstream ssEmpty;
        ssEmpty << "Found empty string at position " << hex << location;
        YUtils::printDebug(ssEmpty.str(),DebugType::WARNING);
        return result;
    }
    // Increment then return offset. 2 birds, meet 1 stone
    while (killOffset < MAX_STRING_LENGTH) {
        // ++ afterwards returns the original value THEN increments
        auto container = bytes.at(location + killOffset);
        if (container == NULL_TERM) {
            return result;
        }
        result += container;
        killOffset++;
    }
    return "STRING LONGER THAN 0xFF";
}

QColor YUtils::getColorFromBytes(uint8_t firstByte, uint8_t secondByte) {
    uint16_t colorBytes = (secondByte << 8) + firstByte;
    uint16_t red = colorBytes & 0b00000'00000'11111;
    uint16_t green = (colorBytes & 0b00000'11111'00000) >> 5;
    uint16_t blue = (colorBytes & 0b11111'00000'00000) >> 10;
    //cout << "red: " << hex << red << ", green: " << green << ", blue: " << blue << endl;
    // (0b11111*8.2 = 254.2)
    auto colRes = QColor(red*8.2,green*8.2,blue*8.2);
    return colRes;
}

std::vector<uint8_t> YUtils::subVector(std::vector<uint8_t> &inVec, uint32_t startOffset, uint32_t endOffset) {
    std::vector<uint8_t> newVec(inVec.begin() + startOffset,inVec.begin() + endOffset);
    return newVec;
}

ChartilePreRenderData YUtils::getCharPreRender(uint16_t tileAttr) {
    ChartilePreRenderData res;
    res.flipV = ((tileAttr >> 11) % 2) == 1;
    res.flipH = ((tileAttr >> 10) % 2) == 1;
    res.paletteId = tileAttr >> 12;
    res.tileId = tileAttr & 0b1111111111;
    res.tileAttr = tileAttr;
    return res;
}

/**
 * @brief Given an uninitialized result vector, combine then insert the two prior vectors into it
 * 
 * @param firstVec 
 * @param secondVec 
 * @param resultVec An uninitialized vector to place concated data in
 */
void YUtils::joinVectors(std::vector<uint8_t> &firstVec, std::vector<uint8_t> &secondVec, std::vector<uint8_t> &resultVec) {
    resultVec.reserve(firstVec.size() + secondVec.size());
    resultVec.insert(resultVec.end(),firstVec.begin(),firstVec.end());
    resultVec.insert(resultVec.end(),secondVec.begin(),secondVec.end());
}

/**
 * @brief Appends the second vector to the end of the first one
 * 
 * @param baseVec 
 * @param appendedVec 
 */
void YUtils::appendVector(std::vector<uint8_t> &baseVec, std::vector<uint8_t> &appendedVec) {
    baseVec.reserve(baseVec.size() + appendedVec.size());
    baseVec.insert(baseVec.end(),appendedVec.begin(),appendedVec.end());
}

std::vector<uint8_t> YUtils::createInstructionVector(std::vector<uint8_t> &instructionVector, std::vector<uint8_t> &data) {
    if (instructionVector.size() != 4) {
        cerr << "[ERROR] Instruction vector was not 4 bytes! Instead got " << hex << instructionVector.size() << endl;
        exit(EXIT_FAILURE);
    }
    uint32_t sizeOfData = data.size();
    // Take sizeOfData, turn it into sizeVector
    uint8_t firstByte = ((sizeOfData % 0x100) >> 0);
    uint8_t secondByte = ((sizeOfData % 0x10000) >> 8);
    uint8_t thirdByte = ((sizeOfData % 0x1000000) >> 16);
    uint8_t fourthByte = ((sizeOfData % 0x100000000) >> 24);
    std::vector<uint8_t> sizeVector = {firstByte,secondByte,thirdByte,fourthByte};

    // Create, join, and return result
    std::vector<uint8_t> result;
    YUtils::joinVectors(instructionVector,sizeVector,result);
    YUtils::appendVector(result,data);
    return result;
}

std::vector<uint8_t> YUtils::createInstVecFromNum(uint32_t instCode, std::vector<uint8_t> &data) {
    uint8_t firstByte = ((instCode % 0x100) >> 0);
    uint8_t secondByte = ((instCode % 0x10000) >> 8);
    uint8_t thirdByte = ((instCode % 0x1000000) >> 16);
    uint8_t fourthByte = ((instCode % 0x100000000) >> 24);
    std::vector<uint8_t> instVec = {firstByte,secondByte,thirdByte,fourthByte};
    return YUtils::createInstructionVector(instVec,data);
}

void YUtils::printVector(std::vector<uint8_t> &vectorToPrint, int newlineBreak) {
    uint32_t lengthOfVec = vectorToPrint.size();
    constexpr int INDEX_WIDTH = 6;
    cout << hex << setw(INDEX_WIDTH) << 0 << " | ";
    uint32_t printIndex = 0;
    for (auto it = vectorToPrint.begin(); it != vectorToPrint.end(); it++) {
        cout << hex << setw(2) << (int)(*it) << " ";
        if (newlineBreak != 0 && printIndex != 0 && (printIndex + 1) % newlineBreak == 0) {
            cout << endl;
            if (printIndex + 1 != lengthOfVec) {
                cout << hex << setw(INDEX_WIDTH) << (printIndex+1) << " | ";
            }
        }
        printIndex++;
    }
    cout << endl;
}

void YUtils::writeVectorToFile(std::vector<uint8_t> &dataToWrite,std::string fileOnSystem,uint32_t addressOffset) {
    std::fstream readWriteFile{fileOnSystem,ios::binary | ios::in | ios::out};
    if (!readWriteFile) {
        cerr << "[ERROR] Failed to write vector to file '" << fileOnSystem << "'" << endl;
        exit(EXIT_FAILURE);
    }
    readWriteFile.seekp(addressOffset);
    readWriteFile.write(reinterpret_cast<char *>(dataToWrite.data()),dataToWrite.size());
    readWriteFile.close();
}

/**
 * @brief Takes in the addresses seen in Ghidra or
 * No$GBA and outputs uncomped ROM-relative address
 * 
 * @param x2address 0x02xxxxxx
 * @return File-relative address
 */
Address YUtils::conv2xAddrToFileAddr(AddressMemory x2address) {
    return x2address - Constants::MAIN_MEM_OFFSET + Constants::ARM9_ROM_OFFSET + 0x4000;
}

void YUtils::printLevelObject(LevelObject lo) {
    cout << "{ id: " << hex << setw(3) << lo.objectId << ", len: " << hex << setw(2) <<
        lo.settingsLength << ", x: " << hex << setw(3) << lo.xPosition <<
        ", y: " << hex << setw(3) << lo.yPosition << " }" << endl;
}

QByteArray YUtils::tileVectorToQByteArray(std::vector<uint8_t> tileVector) {
    QByteArray qb;
    const uint32_t tileVectorSize = tileVector.size();
    if (tileVectorSize != Constants::CHARTILE_DATA_SIZE) {
        std::cerr << "[ERROR] input vector not 0x20 in length: " << hex << tileVectorSize << endl;
        return qb;
    }
    qb.resize(64);
    for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
        uint8_t curByte = tileVector.at(currentTileBuildIndex);
        uint8_t highBit = curByte >> 4;
        uint8_t lowBit = curByte % 0x10;
        int innerPosition = currentTileBuildIndex*2;
        qb[innerPosition+1] = highBit;
        qb[innerPosition+0] = lowBit;
    }
    return qb;
}

int16_t YUtils::roundI16Down(int16_t unrounded, int16_t multiple) {
    if (multiple == 0) return unrounded;

    int16_t remainder = unrounded % multiple;
    if (remainder == 0) return unrounded;

    return (unrounded) - multiple - remainder;
}

void YUtils::printDebug(std::string msg, DebugType dt) {
    switch(dt) {
        case DebugType::VERBOSE: {
            std::cout << "[INFO] " << msg << std::endl;
            break;
        }
        case DebugType::WARNING: {
            std::cout << "[WARN] " << msg << std::endl;
            break;
        }
        case DebugType::ERROR: {
            std::cerr << "[ERROR] " << msg << std::endl;
            break;
        }
        case DebugType::FATAL: {
            std::cerr << "[FATAL] " << msg << std::endl;
            break;
        }
    }
}

std::vector<uint8_t> YUtils::uint32toVec(uint32_t inputInt) {
    std::vector<uint8_t> result;

    uint8_t byte1 = (uint8_t)((inputInt >> 0) % 0x100);
    uint8_t byte2 = (uint8_t)((inputInt >> 8) % 0x100);
    uint8_t byte3 = (uint8_t)((inputInt >> 16) % 0x100);
    uint8_t byte4 = (uint8_t)((inputInt >> 24) % 0x100);

    result.push_back(byte1);
    result.push_back(byte2);
    result.push_back(byte3);
    result.push_back(byte4);

    return result;
}

std::vector<uint8_t> YUtils::uint16toVec(uint16_t inputInt) {
    std::vector<uint8_t> result;

    uint8_t byte1 = (uint8_t)((inputInt >> 0) % 0x100);
    uint8_t byte2 = (uint8_t)((inputInt >> 8) % 0x100);

    result.push_back(byte1);
    result.push_back(byte2);

    return result;
}