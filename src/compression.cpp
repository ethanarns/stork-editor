#include "compression.h"
#include "utils.h"
#include "cue_lzss.h"
#include "cue_blz.h"
#include "GlobalSettings.h"

// std::cerr, std::endl, std::ios
#include <iostream>
// filesystem::current_path
#include <filesystem>
// std::tolower

#include <vector>

// Q_UNUSED
#include <QtGlobal>

const char* NDSTOOL_PATH = "lib/ndstool";
const char* NDSTOOL_PATH_WIN = "lib\\ndstool.exe";

/**
 * @brief Modifies the file in place with Backwards Decompression (BLZ)
 * 
 * @param filepath File to be modified
 * @return True if succeeded, false if failed
 */
bool YCompression::blzDecompress(std::string filepath) {
    using namespace std;
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    auto inFile = filepath.c_str();
    Blz::decode(const_cast<char*>(inFile));
    return true;
}

bool YCompression::lzssRecomp(std::string filepath) {
    using namespace std;
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    auto inFile = filepath.c_str();
    Lzss::encode(const_cast<char*>(inFile));
    return true;
}

std::vector<uint8_t> YCompression::lzssVectorRecomp(std::vector<uint8_t>& uncompressedVec) {
    const std::string tempName = "TEMP_RECOMP.lz10";
    YUtils::writeByteVectorToFile(uncompressedVec,tempName);
    bool recompResult = YCompression::lzssRecomp(tempName);
    if (!recompResult) {
        YUtils::popupAlert("Failed to recompress to LZ77 vector");
        exit(EXIT_FAILURE);
    }

    std::ifstream recomped{tempName, std::ios::binary};
    if (!recomped) {
        YUtils::popupAlert("Failed to load LZ77 recompression file");
        exit(EXIT_FAILURE);
    }

    size_t recomped_length = YUtils::getStreamLength(recomped);
    std::vector<uint8_t> recompVec;
    recompVec.reserve(recomped_length);
    std::copy(
        std::istreambuf_iterator<char>(recomped),
        std::istreambuf_iterator<char>(),
        std::back_inserter(recompVec)
    );
    // Cleanup
    recomped.close();
    std::filesystem::remove(tempName);
    return recompVec;
}

std::filesystem::path YCompression::getAbsoluteRomPart(std::string dataName) {
    std::string dataPath = "./"; // "." means current directory, even within a greater path
    dataPath = dataPath.append(globalSettings.extractFolderName).append("/").append(dataName);
    return YUtils::relativeToEscapedAbs(dataPath);
}

void YCompression::unpackRom(std::string romFileName) {
    std::string toolPath = NDSTOOL_PATH;
    #ifdef _WIN32
        YUtils::printDebug("Switching NDSTool unpack to Windows mode");
        toolPath = NDSTOOL_PATH_WIN;
    #endif

    auto execPath = toolPath;
    if (!std::filesystem::exists(execPath)) {
        YUtils::popupAlert("NDSTool not found");
        exit(EXIT_FAILURE);
    }

    if (std::filesystem::exists(globalSettings.extractFolderName)) {
        YUtils::printDebug("ROM unpack directory already exists, skipping extraction",DebugType::VERBOSE);
        return;
    } else {
        YUtils::printDebug("Unpacking ROM with NDSTool",DebugType::VERBOSE);
        std::filesystem::create_directory(globalSettings.extractFolderName);
    }

    execPath = execPath.append(" -x ").append(YUtils::relativeToEscapedAbs(romFileName));

    auto arm9 = YCompression::getAbsoluteRomPart("arm9.bin");
    execPath = execPath.append(" -9 ").append(arm9.string());

    auto y9 = YCompression::getAbsoluteRomPart("y9.bin");
    execPath = execPath.append(" -y9 ").append(y9.string());

    auto dataDir = YCompression::getAbsoluteRomPart("data/");
    execPath = execPath.append(" -d ").append(dataDir.string());

    auto header = YCompression::getAbsoluteRomPart("header.bin");
    execPath = execPath.append(" -h ").append(header.string());

    auto arm7 = YCompression::getAbsoluteRomPart("arm7.bin");
    execPath = execPath.append(" -7 ").append(arm7.string());

    auto y7 = YCompression::getAbsoluteRomPart("y7.bin");
    execPath = execPath.append(" -y7 ").append(y7.string());

    auto overlays = YCompression::getAbsoluteRomPart("overlay/");
    execPath = execPath.append(" -y ").append(overlays.string());

    auto banner = YCompression::getAbsoluteRomPart("banner.bin");
    execPath = execPath.append(" -t ").append(banner.string());
#ifdef _WIN32
    // Can CMD be silenced in one line?
#else
    execPath.append(" 1> /dev/null");
#endif
    YUtils::printDebug(execPath,DebugType::VERBOSE);
    auto result = system(execPath.c_str());
    if (result == 0) {
        YUtils::printDebug("Command executed successfully",DebugType::VERBOSE);
    } else {
        std::stringstream ssUnpackResult;
        ssUnpackResult << "NDSTool unpack command failed, system result: " << result;
        YUtils::popupAlert(ssUnpackResult.str());
        exit(EXIT_FAILURE);
    }
}

void YCompression::repackRom(std::string outputFileName) {
    std::string toolPath = NDSTOOL_PATH;
#ifdef _WIN32
    YUtils::printDebug("Switching NDSTool unpack to Windows mode");
    toolPath = NDSTOOL_PATH_WIN;
#endif

    auto execPath = toolPath;
    if (!std::filesystem::exists(execPath)) {
        YUtils::popupAlert("NDSTool not found");
        exit(EXIT_FAILURE);
    }

    if (!std::filesystem::exists(globalSettings.extractFolderName)) {
        YUtils::popupAlert("ROM unpack directory not found, canceling repack");
        return;
    }
    if (std::filesystem::exists(outputFileName)) {
        std::filesystem::remove(outputFileName);
    }
    execPath = execPath.append(" -c ").append(YUtils::relativeToEscapedAbs(outputFileName));

    auto arm9 = YCompression::getAbsoluteRomPart("arm9.bin");
    execPath = execPath.append(" -9 ").append(arm9.string());

    auto y9 = YCompression::getAbsoluteRomPart("y9.bin");
    execPath = execPath.append(" -y9 ").append(y9.string());

    auto dataDir = YCompression::getAbsoluteRomPart("data/");
    execPath = execPath.append(" -d ").append(dataDir.string());

    auto header = YCompression::getAbsoluteRomPart("header.bin");
    execPath = execPath.append(" -h ").append(header.string());

    auto arm7 = YCompression::getAbsoluteRomPart("arm7.bin");
    execPath = execPath.append(" -7 ").append(arm7.string());

    auto y7 = YCompression::getAbsoluteRomPart("y7.bin");
    execPath = execPath.append(" -y7 ").append(y7.string());

    auto overlay = YCompression::getAbsoluteRomPart("overlay/");
    execPath = execPath.append(" -y ").append(overlay.string());

    auto banner = YCompression::getAbsoluteRomPart("banner.bin");
    execPath = execPath.append(" -t ").append(banner.string());
#ifdef _WIN32
    // Can it be silenced on the end line?
#else
    execPath.append(" 1> /dev/null");
#endif
    YUtils::printDebug(execPath,DebugType::VERBOSE);
    auto result = system(execPath.c_str());
    if (result == 0) {
        YUtils::printDebug("Command executed successfully",DebugType::VERBOSE);
    } else {
        std::stringstream ssRepackResult;
        ssRepackResult << "NDSTool repack command failed, system result: " << result;
        YUtils::popupAlert(ssRepackResult.str());
        exit(EXIT_FAILURE);
    }
}

// http://problemkaputt.de/gbatek.htm#lzdecompressionfunctions
std::vector<uint8_t> YCompression::lz10decomp(std::vector<uint8_t> data) {
    if (data.at(0) != 0x10) {
        YUtils::popupAlert("Data is not lz10 compressed");
        return std::vector<uint8_t>();
    }
    // Header size is 3 bytes, skipping first which is 0x10
    uint32_t size = YUtils::getUint32FromVec(data,0) >> 8;
    uint32_t src = 4;
    uint32_t dst = 0;

    std::vector<uint8_t> result;
    for (uint x = 0; x < size; x++) {
        result.push_back(0x0);
    }

    while (true) {
        if (dst >= size)
            return result;
        // Flags for next 8 sections
        uint16_t flags = data.at(src++);

        for (uint32_t i = 0; i < 8; i++) {
            if (dst >= size)
                return result;

            // Get the next flag
            if ((flags <<= 1) & (1 << (8))) {
                // This is still kinda confusing...
                uint8_t val1 = data.at(src++);
                uint8_t val2 = data.at(src++);
                uint8_t size = 3 + ((val1 >> 4) & 0xF);
                uint16_t offset = 1 + ((val1 & 0xF) << 8) + val2;

                // Repeat
                for (uint32_t j = 0; j < size; j++) {
                    uint8_t value = result.at(dst - offset);
                    result[dst++] = value;
                }
            } else {
                // Copy from src to dest
                uint8_t value = data.at(src++);
                result[dst++] = value;
            }
        }
    }
}
