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
        cout << "[WARN] Found empty string at position " << hex << location << endl;
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

std::vector<uint8_t> YUtils::subVector(std::vector<uint8_t> inVec, uint32_t startOffset, uint32_t endOffset) {
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
    uint32_t index = 0;
    for (auto it = vectorToPrint.begin(); it != vectorToPrint.end(); it++) {
        cout << hex << setw(2) << (int)*it << " ";
        if (newlineBreak != 0 && index != 0 && index % newlineBreak == 0) {
            cout << endl;
        }
        index++;
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