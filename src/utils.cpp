#include "utils.h"
#include "constants.h"
#include "Chartile.h"
#include "PixelDelegateEnums.h"

// std::cerr, std::endl, std::ios
#include <iostream>
// filesystem::current_path
#include <filesystem>
// std::tolower
#include <cctype>

#include <vector>
#include <fstream>
#include <iomanip>
#include <random>

#include <QtCore>
#include <QColor>
#include <QMessageBox>
#include <QApplication>

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
    using namespace std;
    for (const auto &entry : filesystem::directory_iterator(path)) {
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
    using namespace std;
    std::ofstream outfile(filename, ios::out | ios::binary);
    outfile.write((const char*)&bytes[0], bytes.size());
    outfile.close();
    return true;
}

std::vector<uint8_t> YUtils::getUint8VectorFromFile(std::string fileToLoad) {
    std::vector<uint8_t> vec;
    std::ifstream inputFile{fileToLoad, std::ios::binary};
    std::copy(
        std::istreambuf_iterator<char>(inputFile),
        std::istreambuf_iterator<char>(),
        std::back_inserter(vec)
    );
    return vec;
}

uint32_t YUtils::getUint32FromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    return (uint32_t)(bytes.at(location+3) << 24) + (uint32_t)(bytes.at(location+2) << 16) + (uint32_t)(bytes.at(location+1) << 8) + (uint32_t)bytes.at(location);
}

uint16_t YUtils::getUint16FromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    return (uint16_t)(bytes.at(location+1) << 8) + (uint16_t)bytes.at(location);
}

int16_t YUtils::getSint16FromVec(std::vector<uint8_t> &bytes, uint32_t location) {
    auto u = YUtils::getUint16FromVec(bytes,location);
    return static_cast<int16_t>(u);
}

size_t YUtils::getStreamLength(std::ifstream& buffer) {
    using namespace std;
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
        ssEmpty << "Found empty string at position " << std::hex << location;
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

std::string YUtils::getFixedTextFromVec(std::vector<uint8_t> &bytes, uint32_t location, uint32_t length) {
    std::stringstream ss;
    for (uint i = 0; i < length; i++) {
        ss << bytes.at(i+location);
    }
    return ss.str();
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

MapTileRecordData YUtils::getMapTileRecordDataFromShort(uint16_t mapTileAttr, BgColorMode bgColorMode) {
    Q_UNUSED(bgColorMode);
    MapTileRecordData res;
    res.tileAttr = mapTileAttr;
    // See: http://problemkaputt.de/gbatek.htm#lcdvrambgscreendataformatbgmap
    res.flipV = ((mapTileAttr >> 11) % 2) == 1;
    res.flipH = ((mapTileAttr >> 10) % 2) == 1;
    res.paletteId = mapTileAttr >> 12;
    res.tileId = mapTileAttr & 0b1111111111;
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
        std::cerr << "[ERROR] Instruction vector was not 4 bytes! Instead got " << std::hex << instructionVector.size() << std::endl;
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
    using namespace std;
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

void YUtils::printVector16(std::vector<uint16_t> &vectorToPrint, int newlineBreak) {
    using namespace std;
    std::cout << "printVector16 start" << std::endl;
    uint32_t lengthOfVec = vectorToPrint.size();
    constexpr int INDEX_WIDTH = 6;
    cout << hex << setw(INDEX_WIDTH) << 0 << " | ";
    uint32_t printIndex = 0;
    for (auto it = vectorToPrint.begin(); it != vectorToPrint.end(); it++) {
        cout << hex << setw(4) << (int)(*it) << " ";
        if (newlineBreak != 0 && printIndex != 0 && (printIndex + 1) % newlineBreak == 0) {
            cout << endl;
            if (printIndex + 1 != lengthOfVec) {
                cout << hex << setw(INDEX_WIDTH) << (printIndex+1) << " | ";
            }
        }
        printIndex++;
    }
    cout << endl;
    std::cout << "printVector16 end" << std::endl;
}

void YUtils::writeVectorToFile(std::vector<uint8_t> &dataToWrite,std::string fileOnSystem,uint32_t addressOffset) {
    using namespace std;
    std::fstream readWriteFile{fileOnSystem,ios::binary | ios::out};
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
    using namespace std;
    cout << "{ id: " << hex << setw(3) << lo.objectId << ", len: " << hex << setw(2) <<
        lo.settingsLength << ", x: " << hex << setw(3) << lo.xPosition <<
        ", y: " << hex << setw(3) << lo.yPosition << " }" << endl;
}

std::vector<uint8_t> YUtils::compileObject(LevelObject lo) {
    std::vector<uint8_t> result = YUtils::uint16toVec(lo.objectId);
    auto length = YUtils::uint16toVec(lo.settingsLength);
    YUtils::appendVector(result,length);
    auto xpos = YUtils::uint16toVec(lo.xPosition);
    YUtils::appendVector(result,xpos);
    auto ypos = YUtils::uint16toVec(lo.yPosition);
    YUtils::appendVector(result,ypos);
    YUtils::appendVector(result,lo.settings);
    return result;
}

QByteArray YUtils::tileVectorToQByteArray(std::vector<uint8_t> tileVector) {
    QByteArray qb;
    const uint32_t tileVectorSize = tileVector.size();
    if (tileVectorSize != Constants::CHARTILE_DATA_SIZE) {
        std::cerr << "[ERROR] input vector not 0x20 in length: " << std::hex << tileVectorSize << std::endl;
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

#ifdef _WIN32
// Global, but only for this file
std::ofstream logFile("stork.log", std::ios::app);
void YUtils::printDebug(std::string msg, DebugType dt) {
    switch(dt) {
        case DebugType::VERBOSE: {
            logFile << "[INFO] " << msg << std::endl;
            break;
        }
        case DebugType::WARNING: {
            logFile << "[WARN] " << msg << std::endl;
            break;
        }
        case DebugType::ERROR: {
            logFile << "[ERROR] " << msg << std::endl;
            break;
        }
        case DebugType::FATAL: {
            logFile << "[FATAL] " << msg << std::endl;
            break;
        }
    }
    //logFile.close();
}
#else
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
#endif

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

std::vector<uint8_t> YUtils::stringToVector(std::string &inputString) {
    std::vector<uint8_t> result;
    for (uint8_t c : inputString) {
        result.push_back(c);
    }
    result.push_back(0); // Null terminator
    return result;
}

void YUtils::printQbyte(QByteArray& qb, int newlineBreak) {
    using namespace std;
    uint32_t lengthOfQ = qb.size();
    auto qbArray = (uint8_t*)qb.data();
    constexpr int INDEX_WIDTH = 6;
    cout << hex << setw(INDEX_WIDTH) << 0 << " | ";
    for (uint32_t qIndex = 0; qIndex < lengthOfQ; qIndex++) {
        auto it = qbArray[qIndex];
        cout << hex << setw(2) << (uint16_t)(it) << " ";
        if (newlineBreak != 0 && qIndex != 0 && (qIndex + 1) % newlineBreak == 0) {
            cout << endl;
            if (qIndex + 1 != lengthOfQ) {
                cout << hex << setw(INDEX_WIDTH) << (qIndex+1) << " | ";
            }
        }
    }
    cout << endl;
}

void YUtils::popupAlert(std::string msg) {
    QMessageBox::warning( 
        QApplication::activeWindow(), 
        "Stork Editor",
        msg.c_str()
    );
}

CollisionMetadata YUtils::getCollisionMetadata(CollisionType colType) {
    CollisionMetadata result;
    // Defaults
    result.prettyName = "UNKNOWN";
    result.colType = CollisionType::NONE;
    result.preview = CollisionDraw::SQERR;
    switch (colType) {
        case CollisionType::NONE: {
            result.prettyName = "Clear/Erase";
            result.preview = CollisionDraw::CLEAR;
            break;
        }
        case CollisionType::SQUARE: {
            result.prettyName = "Solid Square";
            result.preview = CollisionDraw::SQUARE_DRAW;
            break;
        }
        case CollisionType::PLATFORM_PASSABLE: {
            result.prettyName = "Upwards Passable";
            result.preview = CollisionDraw::ZIG_ZAG;
            break;
        }
        case CollisionType::UP_RIGHT_30: {
            result.prettyName = "Up Right 30 1";
            result.preview = CollisionDraw::UP_RIGHT_30_BL;
            break;
        }
        case CollisionType::UP_RIGHT_30_HALFSTART: {
            result.prettyName = "Up Right 30 2";
            result.preview = CollisionDraw::UP_RIGHT_30_BR;
            break;
        }
        case CollisionType::UP_RIGHT_STEEP_1: {
            result.prettyName = "Up Right Steep 1";
            result.preview = CollisionDraw::UP_RIGHT_STEEP_SHORT;
            break;
        }
        case CollisionType::UP_RIGHT_STEEP_2: {
            result.prettyName = "Up Right Steep 2";
            result.preview = CollisionDraw::UP_RIGHT_STEEP_TALL;
            break;
        }
        case CollisionType::UP_RIGHT_45: {
            result.prettyName = "Up Right 45";
            result.preview = CollisionDraw::DIAG_UP_RIGHT;
            break;
        }
        case CollisionType::STATIC_COIN: {
            result.prettyName = "Static Coin";
            // This could be better... Currently clear, but top right overflows
            result.preview = CollisionDraw::COIN_BOTTOM_RIGHT;
            break;
        }
        case CollisionType::DOWN_RIGHT_30_2: {
            result.prettyName = "Down Right 30 2";
            result.preview = CollisionDraw::DOWN_RIGHT_30_SHORT;
            break;
        }
        case CollisionType::DOWN_RIGHT_30_1: {
            result.prettyName = "Down Right 30 1";
            result.preview = CollisionDraw::DOWN_RIGHT_30_TALL;
            break;
        }
        case CollisionType::DOWN_RIGHT_STEEP: {
            result.prettyName = "Down Right Steep 1";
            result.preview = CollisionDraw::DOWN_RIGHT_STEEP_THIN;
            break;
        }
        case CollisionType::DOWN_RIGHT_STEEP_HALFSTART: {
            result.prettyName = "Down Right Steep 2";
            result.preview = CollisionDraw::DOWN_RIGHT_STEEP_WIDE;
            break;
        }
        case CollisionType::DOWN_RIGHT_45: {
            result.prettyName = "Down Right 45";
            result.preview = CollisionDraw::DIAG_DOWN_RIGHT;
            break;
        }
        case CollisionType::KILL_SPIKES: {
            result.prettyName = "Instant Kill";
            result.preview = CollisionDraw::ZIG_ZAG_UPSIDE_DOWN_RED;
            break;
        }
        case CollisionType::ICY_SQUARE: {
            result.prettyName = "Slippery Ice";
            result.preview = CollisionDraw::SLIPPERY_ICE_SQUARE;
            break;
        }
        case CollisionType::SOFT_ROCK: {
            result.prettyName = "Soft Rock";
            result.preview = CollisionDraw::SOFT_ROCK_SQUARE;
            break;
        }
        case CollisionType::UPSIDE_DOWN_SLOPE_30_1: {
            result.prettyName = "Upside Down 30 1";
            result.preview = CollisionDraw::UPSIDE_DOWN_SLOPE_30_1_DRAW;
            break;
        }
        case CollisionType::UPSIDE_DOWN_SLOPE_30_2: {
            result.prettyName = "Upside Down 30 2";
            result.preview = CollisionDraw::UPSIDE_DOWN_SLOPE_30_2_DRAW;
            break;
        }
        case CollisionType::CLIMBABLE_VINE_CEILING: {
            result.prettyName = "Climbable Vine (Ceiling)";
            result.preview = CollisionDraw::VINE_LEFT;
            break;
        }
        case CollisionType::CLIMBABLE_VINE_TALL: {
            result.prettyName = "Climbable Vine";
            result.preview = CollisionDraw::VINE_RIGHT;
            break;
        }
        // Next 3 have been tested and confirmed
        case CollisionType::UPSIDE_DOWN_DOWNWARDS_45: {
            result.prettyName = "Upside-Down Down 45";
            result.preview = CollisionDraw::UPSIDE_DOWN_DOWNWARDS_45_DRAW;
            break;
        }
        case CollisionType::UPSIDE_DOWN_UP_30: {
            result.prettyName = "Upside-Down Up 30 1";
            result.preview = CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_SHORT;
            break;
        }
        case CollisionType::UPSIDE_DOWN_UP_30_2: {
            result.prettyName = "Upside-Down Up 30 2";
            result.preview = CollisionDraw::UPSIDE_DOWN_RIGHT_UP_30_TALL;
            break;
        }
        case CollisionType::UPSIDE_DOWN_UP_RIGHT: {
            result.prettyName = "Upside-Down Up Right 45";
            result.preview = CollisionDraw::UPSIDE_DOWN_RIGHT_45;
            break;
        }
        case CollisionType::STAIRS_DOWN_RIGHT: {
            result.prettyName = "Stairs Down";
            result.preview = CollisionDraw::STAIRS_DOWN_RIGHT_DRAW;
            break;
        }
        case CollisionType::WATER_STILL: {
            result.prettyName = "Still Water";
            result.preview = CollisionDraw::WATER_STILL_DRAW;
            break;
        }
        case CollisionType::LAVA_KILL: {
            result.prettyName = "Lava";
            result.preview = CollisionDraw::LAVA_KILL_DRAW;
            break;
        }
        case CollisionType::UPSIDE_DOWN_SHARP_UP_1: {
            result.prettyName = "Upside-Down Up Sharp 1";
            result.preview = CollisionDraw::UPSIDE_DOWN_SHARP_UP_SHORT_DRAW;
            break;
        }
        case CollisionType::UPSIDE_DOWN_SHARP_UP_2: {
            result.prettyName = "Upside-Down Up Sharp 2";
            result.preview = CollisionDraw::UPSIDE_DOWN_SHARP_UP_TALL_DRAW;
            break;
        }
        case CollisionType::UPSIDE_DOWN_SHARP_DOWN_2: {
            result.prettyName = "Upside-Down Down Sharp 2";
            result.preview = CollisionDraw::UPSIDE_DOWN_SHARP_DOWN_TALL_DRAW;
            break;
        }
        case CollisionType::UPSIDE_DOWN_SHARP_DOWN_1: {
            result.prettyName = "Upside-Down Down Sharp 1";
            result.preview = CollisionDraw::UPSIDE_DOWN_SHARP_DOWN_SHORT_DRAW;
            break;
        }
        // Note: missing stairs up, but coldraw for it is already there
    }
    return result;
}

QColor YUtils::invertColor(QColor in) {
    QColor newColor;
    newColor.setRed(0xff-in.red());
    newColor.setGreen(0xff-in.green());
    newColor.setBlue(0xff-in.blue());
    return newColor;
}

std::string YUtils::magicToAscii(uint32_t hexText) {
    uint32_t lastByte = hexText >> 24;
    uint32_t thirdByte = hexText >> 16 % 0x100;
    uint32_t secondByte = hexText >> 8 % 0x100;
    uint32_t firstByte = hexText % 0x100;
    std::string result;
    result.push_back((char)firstByte);
    result.push_back((char)secondByte);
    result.push_back((char)thirdByte);
    result.push_back((char)lastByte);
    return result;
}

// https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
std::string YUtils::generateUuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0,15);
    std::uniform_int_distribution<> dis2(8,11);
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    };
    return ss.str();
}

std::string YUtils::musicIdToText(uint8_t musicId) {
    // https://www.youtube.com/watch?v=Zb9jvSQg9sw
    // To find these, break on 2013208, load 1-1, and edit 0231e307
    // TODO: See which of these are unused in CSCNs
    switch(musicId) {
        case 0x00: return "Flower Garden (duplicate?)";
        case 0x01: return "Story Music Box";
        case 0x02: return "Yoshi's Island DS";
        case 0x03: return "Flower Field";
        case 0x04: return "Yoshi's Island DS (duplicate?)";
        case 0x05: return "Yoshi's Island DS (duplicate?)";
        case 0x06: return "Training Course";
        case 0x07: return "Score";
        case 0x08: return "Minigame";
        case 0x09: return "Flower Garden";
        case 0x0A: return "Underground";
        case 0x0B: return "Sea Coast";
        case 0x0C: return "Jungle";
        case 0x0D: return "Castle";
        case 0x0E: return "In the Clouds";
        case 0x0F: return "Wildlands";
        case 0x10: return "Bonus Challenge";
        case 0x11: return "Kamek's Theme";
        case 0x12: return "Mini-Boss";
        case 0x13: return "Boss Room";
        case 0x14: return "Big Boss";
        case 0x15: return "Flower Garden (duplicate?)";
        case 0x16: return "Bowser";
        case 0x17: return "Castle (duplicate?)";
        case 0x18: return "Silence";
        case 0x19: return "Silence (Echoes)";
        // 0x1A, 1b, just repeats silence and echoes
        default: return "Unhandled";
    }
}

QPoint YUtils::getSpriteDimsFromFlagValue(uint flagValue) {
    // Width is X, Height is Y
    switch (flagValue) {
        case 0x0:  return QPoint(1,1);
        case 0x1:  return QPoint(2,2);
        case 0x2:  return QPoint(4,4);
        case 0x3:  return QPoint(8,8);
        case 0x4:  return QPoint(1,1);
        case 0x5:  return QPoint(2,2);
        case 0x6:  return QPoint(4,4);
        case 0x7:  return QPoint(8,8);
        case 0x8:  return QPoint(2,1);
        case 0x9:  return QPoint(4,1);
        case 0xA:  return QPoint(4,2);
        case 0xB:  return QPoint(8,4);
        case 0xC:  return QPoint(2,1);
        case 0xD:  return QPoint(4,1);
        case 0xE:  return QPoint(4,2);
        case 0xF:  return QPoint(8,4);
        case 0x10: return QPoint(1,2);
        case 0x11: return QPoint(1,4);
        case 0x12: return QPoint(2,4);
        case 0x13: return QPoint(4,8);
        case 0x14: return QPoint(1,2);
        default:   break;
    }
    std::stringstream ss;
    ss << "Unknown flagValue: 0x" << std::hex << flagValue;
    YUtils::printDebug(ss.str(),DebugType::ERROR);
    return QPoint(3,3);
}

std::string YUtils::relativeToEscapedAbs(std::string relPath) {
    std::stringstream res;
    auto absPath = std::filesystem::absolute(relPath);
    res << std::quoted(absPath.string());
    return res.str();
}
