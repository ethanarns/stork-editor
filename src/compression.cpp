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

std::vector<uint8_t> YCompression::lzssVectorDecomp(std::vector<uint8_t>& inputVec, bool verbose) {
    const std::string tempName = "TEMP.lz10";
    YUtils::writeByteVectorToFile(inputVec,tempName);
    bool decompResult = YCompression::lzssDecomp(tempName, verbose);
    if (!decompResult) {
        cerr << "Failed to extract MPDZ file" << endl;
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