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

const char* ROM_FILENAME = "rom.nds";
const char* BLZ_PATH = "./blz";
const char* LZSS_PATH = "./lzss";

/**
 * @brief Modifies the file in place with Backwards Decompression (BLZ)
 * 
 * @param filepath File to be modified
 * @return True if succeeded, false if failed
 */
bool YCompression::blzDecompress(std::string filepath) {
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    std::string blzPath = BLZ_PATH;
    std::string blzCmd = blzPath.append(" -d ").append(filepath);
    #ifdef DEBUG
    cout << "> " << blzCmd << endl;
    #else
    // Silence stdout
    blzCmd.append(" 1> /dev/null");
    #endif
    auto result = system(blzCmd.c_str());
    cout << "System result: " << endl;
    return true;
}

bool YCompression::lzssDecomp(std::string filepath) {
    if (filepath.size() == 0 || filepath.compare("/") == 0) {
        cerr << "Invalid filepath: " << filepath << endl;
        return false;
    }
    std::string lzssPath = LZSS_PATH;
    std::string lzssCmd = lzssPath.append(" -d ").append(filepath);
    #ifdef DEBUG
    cout << "> " << lzssCmd << endl;
    #else
    // Silence stdout
    lzssCmd.append(" 1> /dev/null");
    #endif
    system(lzssCmd.c_str());
    return true;
}