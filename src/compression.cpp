#include "compression.h"
#include "utils.h"

// std::cerr, std::endl, std::ios
#include <iostream>
// filesystem::current_path
#include <filesystem>
// std::tolower
#include <cctype>

#include <vector>

// Q_UNUSED
#include <QtGlobal>

using namespace std;

const char* BLZ_PATH = "./blz";
const char* LZSS_PATH = "./lzss";
const char* ROM_EXTRACT_DIR = "_nds_unpack";
const char* NDSTOOL_PATH = "./lib/ndstool";

/**
 * @brief Modifies the file in place with Backwards Decompression (BLZ)
 * 
 * @param filepath File to be modified
 * @return True if succeeded, false if failed
 */
bool YCompression::blzDecompress(std::string filepath, bool verbose) {
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    std::string blzPath = BLZ_PATH;
    std::string blzCmd = blzPath.append(" -d ").append(filepath);
    if (verbose) cout << "> " << blzCmd << endl;
    // Silence stdout
    if (!verbose) blzCmd.append(" 1> /dev/null");
    auto result = system(blzCmd.c_str());
    if (verbose) cout << "System result: " << result << endl;
    return true;
}

bool YCompression::lzssDecomp(std::string filepath, bool verbose) {
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    std::string lzssPath = LZSS_PATH;
    std::string lzssCmd = lzssPath.append(" -d ").append(filepath);
    if (verbose) cout << "> " << lzssCmd << endl;
    // Silence stdout?
    if (!verbose) lzssCmd.append(" 1> /dev/null");
    auto result = system(lzssCmd.c_str());
    if (verbose) cout << "System result: " << result << endl;
    return true;
}

bool YCompression::lzssRecomp(std::string filepath, bool verbose) {
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    std::string lzssPath = LZSS_PATH;
    std::string lzssCmdRecomp = lzssPath.append(" -evn ").append(filepath);
    if (verbose) cout << "> " << lzssCmdRecomp << endl;
    // Silence stdout?
    if (!verbose) lzssCmdRecomp.append(" 1> /dev/null");
    auto result = system(lzssCmdRecomp.c_str());
    if (verbose) cout << "System result: " << result << endl;
    return true;
}

std::vector<uint8_t> YCompression::lzssVectorDecomp(std::vector<uint8_t>& inputVec, bool verbose) {
    const std::string tempName = "TEMP.lz10";
    YUtils::writeByteVectorToFile(inputVec,tempName);
    bool decompResult = YCompression::lzssDecomp(tempName, verbose);
    if (!decompResult) {
        cerr << "Failed to extract LZ77 file" << endl;
        exit(EXIT_FAILURE);
    }

    std::ifstream uncomped{tempName, ios::binary};
    if (!uncomped) {
        cerr << "Failed to load uncompressed file '" << tempName << "'" << endl;
        exit(EXIT_FAILURE);
    }

    size_t uncomped_length = YUtils::getStreamLength(uncomped);
    std::vector<uint8_t> uncompVec;
    uncompVec.reserve(uncomped_length);
    std::copy(
        std::istreambuf_iterator<char>(uncomped),
        std::istreambuf_iterator<char>(),
        std::back_inserter(uncompVec)
    );
    // Cleanup
    uncomped.close();
    std::filesystem::remove(tempName);
    return uncompVec;
}

std::vector<uint8_t> YCompression::lzssVectorRecomp(std::vector<uint8_t>& uncompressedVec, bool verbose) {
    const std::string tempName = "TEMP_RECOMP.lz10";
    YUtils::writeByteVectorToFile(uncompressedVec,tempName);
    bool recompResult = YCompression::lzssRecomp(tempName, verbose);
    if (!recompResult) {
        cerr << "Failed to recompress to LZ77 file" << endl;
        exit(EXIT_FAILURE);
    }

    std::ifstream recomped{tempName, ios::binary};
    if (!recomped) {
        cerr << "Failed to load recompressed file '" << tempName << "'" << endl;
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

void YCompression::unpackRom(std::string romFileName) {
    bool windows = false;

    std::string execPath = NDSTOOL_PATH;
    if (windows) {
        execPath = execPath.append(".exe");
    }
    if (!std::filesystem::exists(execPath)) {
        YUtils::printDebug("NDSTool not found",DebugType::FATAL);
        exit(EXIT_FAILURE);
    }
    if (std::filesystem::exists(ROM_EXTRACT_DIR)) {
        YUtils::printDebug("ROM unpack directory already exists, skipping extraction",DebugType::VERBOSE);
        return;
    } else {
        YUtils::printDebug("Unpacking ROM with NDSTool",DebugType::VERBOSE);
        std::filesystem::create_directory(ROM_EXTRACT_DIR);
    }
    execPath = execPath.append(" -x ").append(romFileName);
    execPath = execPath.append(" -9 ").append(ROM_EXTRACT_DIR).append("/arm9.bin");
    execPath = execPath.append(" -y9 ").append(ROM_EXTRACT_DIR).append("/y9.bin");
    execPath = execPath.append(" -d ").append(ROM_EXTRACT_DIR).append("/data");
    execPath = execPath.append(" -h ").append(ROM_EXTRACT_DIR).append("/header.bin");
    execPath = execPath.append(" -7 ").append(ROM_EXTRACT_DIR).append("/arm7.bin");
    execPath = execPath.append(" -y7 ").append(ROM_EXTRACT_DIR).append("/y7.bin");
    execPath = execPath.append(" -y ").append(ROM_EXTRACT_DIR).append("/overlay");
    execPath = execPath.append(" -t ").append(ROM_EXTRACT_DIR).append("/banner.bin");
    if (!windows) execPath.append(" 1> /dev/null");
    YUtils::printDebug(execPath,DebugType::VERBOSE);
    auto result = system(execPath.c_str());
    if (result == 0) {
        YUtils::printDebug("Command executed successfully",DebugType::VERBOSE);
    } else {
        std::stringstream ssUnpackResult;
        ssUnpackResult << "System result: " << result;
        YUtils::printDebug(ssUnpackResult.str(),DebugType::VERBOSE);
    }
}