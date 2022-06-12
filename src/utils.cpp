#include "utils.h"

// std::cerr, std::endl, std::ios
#include <iostream>
// filesystem::current_path
#include <filesystem>
// std::tolower
#include <cctype>

#include <vector>
#include <fstream>

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