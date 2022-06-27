#include "yidsrom.h"
#include "compression.h"
#include "Chartile.h"
#include "utils.h"
#include "constants.h"

#include <iostream>
#include <vector>

using namespace std;

void YidsRom::loadCrsb(std::string fileName_noext) {
    fileName_noext = YUtils::getLowercase(fileName_noext);
    auto fileName = fileName_noext.append(".crsb");
    auto fileId = this->fileIdMap[fileName];
    if (fileId == 0) {
        cerr << "Failed to load level: " << fileName << endl;
        return;
    }
    // x8 because each record of ranges is 8 bytes. 4 + 4
    Address rangesAddr = this->metadata.fatOffsets + fileId * 8;
    Address startAddr = this->getNumberAt<uint32_t>(rangesAddr + 0);
    Address endAddr = this->getNumberAt<uint32_t>(rangesAddr + 4);
    uint32_t length = endAddr - startAddr;
    // 0x08 is from device capacity. Understand this better
    uint32_t MAX_SIZE = 1U << (17 + 0x08);
    if (length > MAX_SIZE) {
        cerr << "File is too big! File size: " << dec << length << ", max size: " << dec << MAX_SIZE << endl;
        return;
    }
    
    std::string magicText = this->getTextAt(startAddr + 0,4);
    if (magicText.compare(Constants::CRSB_MAGIC) != 0) {
        cerr << "Magic header text " << Constants::CRSB_MAGIC << " not found! Found '" << magicText << "' instead." << endl;
        return;
    }
    // Important is MPDZ, which are marked by SCEN, and is the actual data for
    //   each level and sublevel inside a world level. 1-1 has 4, one for each
    //   area, and no more

    // Number of CSCN records, aka maps in level!
    auto mapFileCount = this->getNumberAt<uint32_t>(startAddr + 8);
    if (this->verbose) cout << "Map count: " << mapFileCount << endl;

    const uint32_t OFFSET_TO_FIRST_CSCN = 0xC; // 12

    // This only gets the first one. The block after this gets them all
    Address baseAddrCscn = startAddr + OFFSET_TO_FIRST_CSCN;
    auto mpdzFilename_noext = this->getTextNullTermAt(baseAddrCscn + 0xC);
    this->loadMpdz(mpdzFilename_noext);

    // uint32_t curCscnReadOffset = 0; // In bytes
    // for (uint32_t cscnIndex = 0; cscnIndex < mapFileCount; cscnIndex++) {
    //     // Points to the current magic number text, CSCN
    //     Address baseAddrCscn = startAddr + OFFSET_TO_FIRST_CSCN + curCscnReadOffset;
    //     // Check that the magic text is there, at index 0
    //     std::string magicTextCscn = this->getTextAt(baseAddrCscn + 0, 4);
    //     if (magicTextCscn.compare(Constants::CSCN_MAGIC) != 0) {
    //         cerr << "Magic header text " << Constants::CSCN_MAGIC << " not found! Found '" << magicTextCscn << "' instead." << endl;
    //         return;
    //     }
    //     // Next, get the filename
    //     auto mpdzFilename_noext = this->getTextNullTermAt(baseAddrCscn + 0xC);
    //     this->loadMpdz(mpdzFilename_noext);

    //     // +0x4 is because the magic number is 4 long, and the next is the length
    //     // +0x8 is because that length is added to the current read position in the file
    //     //   That recreates it as a non-relative offset number (it started from the
    //     //   maybeExits at +0x8)
    //     uint32_t cscnLength = this->getNumberAt<uint32_t>(baseAddrCscn + 0x4) + 0x8;
        
    //     curCscnReadOffset += cscnLength;
    //     return;
    // }

    if (this->verbose) cout << "MPDZ files loaded" << endl;
}

void YidsRom::loadMpdz(std::string fileName_noext) {
    if (this->verbose) cout << "Loading MPDZ '" << fileName_noext << "'" << endl;
    std::string mpdzFileName = fileName_noext.append(Constants::MPDZ_EXTENSION);
    auto fileVector = this->getFileByteVector(mpdzFileName);
    // YUtils::writeByteVectorToFile(fileVector,mpdzFileName); // Uncomment to get uncompressed MPDZ
    auto uncompVec = YCompression::lzssVectorDecomp(fileVector,false);
    
    uint32_t magic = YUtils::getUint32FromVec(uncompVec,0);
    if (magic != Constants::MPDZ_MAGIC_NUM) {
        cerr << "MPDZ Magic number not found! Expected " << hex << Constants::MPDZ_MAGIC_NUM;
        cerr << ", got " << hex << magic << " instead." << endl;
        return;
    } else {
        if (this->verbose) cout << "[SUCCESS] MPDZ Magic number found" << endl;
    }
    // 4 because the file length is written at bytes 4-7
    uint32_t mpdzFileLength = YUtils::getUint32FromVec(uncompVec, 4);
    // 8 in order to start it at the first instruction besides SET
    Address mpdzIndex = 8; // Pass this in as a pointer to functions

    // Instruction loop
    while (mpdzIndex < mpdzFileLength) {
        uint32_t curInstruction = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
        if (curInstruction == Constants::SCEN_MAGIC_NUM) {
            this->handleSCEN(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::GRAD_MAGIC_NUM) {
            this->handleGrad(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::SETD_MAGIC_NUM) {
            this->handleSETD(uncompVec,mpdzIndex);
        } else {
            cerr << "[WARN] Instruction besides SCEN used: " << hex << curInstruction << endl;
            return;
        }
    }
    cout << "All instructions for '" << mpdzFileName << "' completed" << endl;
}

uint32_t timesPaletteLoaded = 0;
uint32_t timesImbzLoaded = 0;

/**
 * @brief Handles the SCEN instruction from MPDZ files
 * 
 * @param mpdzVec Reference to vector with MPDZ data, all of it
 * @param indexPointer Reference to Address, pointing at the current SCEN instruction
 */
void YidsRom::handleSCEN(std::vector<uint8_t>& mpdzVec, Address& indexPointer) {
    uint16_t whichBgToWriteTo = 0;
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::SCEN_MAGIC_NUM) {
        cerr << "SCEN instruction did not find magic hex " << hex << Constants::SCEN_MAGIC_NUM << endl;
        return;
    }
    indexPointer += sizeof(uint32_t);
    uint32_t scenLength = YUtils::getUint32FromVec(mpdzVec, indexPointer);
    indexPointer += sizeof(uint32_t);
    const uint32_t scenCutoff = indexPointer + scenLength;
    while (indexPointer < scenCutoff) {
        uint32_t curSubInstruction = YUtils::getUint32FromVec(mpdzVec,indexPointer);
        if (curSubInstruction == Constants::INFO_MAGIC_NUM) {
            cout << ">> Handling INFO instruction: ";
            uint32_t infoLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First time: 0x20

            uint32_t canvasDimensions = YUtils::getUint32FromVec(mpdzVec, indexPointer + 8); // 00b60208
            // Only the first one matters for the primary height and width, since BG 2 decides everything
            if (this->canvasWidth == 0) {
                this->canvasHeight = canvasDimensions >> 0x10;
                this->canvasWidth = canvasDimensions % 0x10000;
            }

            // TODO: What are these values?
            uint32_t unknownValue1 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 12); // 00000000
            cout << "unk1: " << hex << unknownValue1 << "; ";
            uint32_t unknownValue2 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 16); // 0x1000
            cout << "unk2: " << hex << unknownValue2 << "; ";
            uint32_t unknownValue3 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 20); // 0x1000
            cout << "unk3: " << hex << unknownValue3 << "; ";
            //uint32_t unknownValue4 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 24); // 0x00020202
            whichBgToWriteTo = mpdzVec.at(indexPointer + 24 + 0);
            cout << "whichBg: " << (int)whichBgToWriteTo << "; ";
            // uint16_t charBaseBlockHardMaybe = mpdzVec.at(indexPointer + 24 + 1);
            // uint16_t thirdByte = mpdzVec.at(indexPointer + 24 + 2);
            // uint16_t screenBaseBlockMaybe = mpdzVec.at(indexPointer + 24 + 3);

            uint32_t unknownValue5 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 28); // 00000000
            cout << "unk5: " << hex << unknownValue5 << endl;
            Q_UNUSED(unknownValue1);
            Q_UNUSED(unknownValue2);
            Q_UNUSED(unknownValue3);
            Q_UNUSED(unknownValue5);
            if (infoLength > 0x18) {
                // Get charfile string
                auto charFileNoExt = YUtils::getNullTermTextFromVec(mpdzVec, indexPointer + 32);
                if (timesImbzLoaded == 0) this->handleImbz(charFileNoExt);
                timesImbzLoaded++;
            }
            // Increment based on earlier length, +8 is to skip instruction and length
            indexPointer += infoLength + 8;
        } else if (curSubInstruction == Constants::PLTB_MAGIC_NUM) {
            cout << ">> Handling PLTB instruction" << endl;
            uint32_t pltbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            Address pltbReadIndex = indexPointer + 8; // +8 is to skip instruction and length
            indexPointer += pltbLength + 8; // Skip past, don't do a manual count up
            if (timesPaletteLoaded > 0) {
                cout << "[WARN] Only BG palette supported thus far, skipping" << endl;
                continue;
            }
            // Cycle up to the index pointer
            int globalPaletteIndex = 1; // Start at 1 because universal
            while (pltbReadIndex < indexPointer) {
                QByteArray currentLoadingPalette;
                currentLoadingPalette.resize(Constants::PALETTE_SIZE);
                for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
                    currentLoadingPalette[curPaletteIndex] = mpdzVec.at(pltbReadIndex + curPaletteIndex);
                }
                // Should round down because of int
                this->currentPalettes[globalPaletteIndex] = currentLoadingPalette;
                globalPaletteIndex++;
                pltbReadIndex += Constants::PALETTE_SIZE; // 1 palette is 32 bytes, or 0x20
            }
            timesPaletteLoaded++;
        } else if (curSubInstruction == Constants::MPBZ_MAGIC_NUM) {
            cout << ">> Handling MPBZ instruction" << endl;
            if (whichBgToWriteTo == 0) {
                cerr << "[ERROR] Which BG to write to was not specified, MPBZ load failed" << endl;
                return;
            }
            uint32_t mpbzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            // Slice out MPBZ data
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + mpbzLength;
            auto compressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            // Decompress MPBZ data
            auto uncompressedMpbz = YCompression::lzssVectorDecomp(compressedSubArray, false);
            indexPointer += mpbzLength + 8; // Skip ahead main pointer to next

            if (whichBgToWriteTo == 1 || whichBgToWriteTo == 3) {
                cout << "[WARN] MPBZ tiles other than BG 2 not implemented, skipping" << endl;
                continue;
            }

            // Handle uncompressedMpbz data
            const uint32_t uncompressedMpbzTwoByteCount = uncompressedMpbz.size() / 2;
            //for (int mpbzIndex = 0; mpbzIndex < uncompressedMpbzTwoByteCount; mpbzIndex++) {
            for (uint32_t mpbzIndex = 0; mpbzIndex < uncompressedMpbzTwoByteCount; mpbzIndex++) {
                uint32_t trueOffset = mpbzIndex*2;
                uint16_t firstByte = (uint16_t)uncompressedMpbz.at(trueOffset);
                uint16_t secondByte = (uint16_t)uncompressedMpbz.at(trueOffset+1);
                uint16_t curShort = (secondByte << 8) + firstByte;
                curShort += 0x1000; // 0201c730
                if (whichBgToWriteTo == 2) {
                    this->preRenderDataBg2.push_back(curShort);
                } else {
                    // Do nothing
                }
            }
        } else if (curSubInstruction == Constants::COLZ_MAGIC_NUM) {
            cout << ">> Handling COLZ instruction" << endl;
            if (collisionTileArray.size() > 0) {
                cout << "[ERROR] Attempted to load a second COLZ, only one should ever be loaded" << endl;
                exit(EXIT_FAILURE);
            }
            uint32_t colzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First is 0x0b7c
            // Slice out COLZ data
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + colzLength;
            auto colzCompressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            auto uncompressedColz = YCompression::lzssVectorDecomp(colzCompressedSubArray, false);
            YUtils::appendVector(this->collisionTileArray,uncompressedColz);
            indexPointer += colzLength + 8;
        } else if (curSubInstruction == Constants::ANMZ_MAGIC_NUM) {
            cout << ">> Handling ANMZ instruction" << endl;
            uint32_t anmzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // Should be 0x1080 first time
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + anmzLength;
            auto compressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            auto uncompressedAnmz = YCompression::lzssVectorDecomp(compressedSubArray, false);
            // TODO: Do something with this data
            indexPointer += anmzLength + 8; // Go to next
        } else if (curSubInstruction == Constants::IMGB_MAGIC_NUM) {
            cout << ">> Handling IMGB instruction" << endl;
            uint32_t imgbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            // TODO: Learn about this data
            indexPointer += imgbLength + 8;
        } else if (curSubInstruction == Constants::SCEN_MAGIC_NUM) {
            cerr << "[ERROR] Found SCEN instruction, overflowed!" << endl;
            return;
        } else {
            cout << "Unknown instruction: " << hex << curSubInstruction << endl;
            return;
        }
    }
}

void YidsRom::handleImbz(std::string fileName_noext) {
    if (this->verbose) cout << ">> Handling IMBZ file: '" << fileName_noext << "'" << endl;
    auto uncompressedFileVector = this->getFileByteVector(fileName_noext.append(".imbz"));
    std::vector uncompressedImbz = YCompression::lzssVectorDecomp(uncompressedFileVector,true);
    uncompressedFileVector.clear();

    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles. Start at -1 due to first time ++
    int imbzIndex = 0; // Goes up by 0x20 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    // Do it 0x20 by 0x20
    while (imbzIndex < imbzLength) { // Kill when equal to length, meaning it's outside
        Chartile curTile;
        curTile.engine = ScreenEngine::A;
        curTile.index = currentTileIndex;
        curTile.tiles.resize(64);
        // Go up by 2 since you split the bytes
        for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
            uint8_t curByte = uncompressedImbz.at(imbzIndex + currentTileBuildIndex);
            uint8_t highBit = curByte >> 4;
            uint8_t lowBit = curByte % 0x10;
            int innerPosition = currentTileBuildIndex*2;
            curTile.tiles[innerPosition+1] = highBit;
            curTile.tiles[innerPosition+0] = lowBit;
        }
        this->pixelTiles.push_back(curTile);
        // Skip ahead by 0x20
        imbzIndex += Constants::CHARTILE_DATA_SIZE;
        currentTileIndex++;
    }
}

void YidsRom::handleGrad(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::GRAD_MAGIC_NUM) {
        cerr << "GRAD instruction did not find magic hex " << hex << Constants::GRAD_MAGIC_NUM << endl;
        return;
    }
    cout << "*** Starting GRAD instruction parse ***" << endl;
    indexPointer += 4; // Go to length
    auto gradLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    indexPointer += 4; // Now at start of actual data

    // Do stuff here
    // For now, skip
    indexPointer += gradLength;
}

void YidsRom::handleSETD(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::SETD_MAGIC_NUM) {
        cerr << "SETD instruction did not find magic hex " << hex << Constants::SETD_MAGIC_NUM << endl;
        return;
    }
    cout << "*** Starting SETD instruction parse ***" << endl;
    indexPointer += 4; // Go to length
    auto setdLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    // Now at start of actual data
    indexPointer += 4;

    // Do stuff here
    // For now, skip
    indexPointer += setdLength;
}