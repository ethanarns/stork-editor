#include "yidsrom.h"
#include "compression.h"
#include "Chartile.h"
#include "utils.h"
#include "constants.h"
#include "LevelObject.h"
#include "PixelDelegate.h"

#include <iostream>
#include <vector>

using namespace std;

void YidsRom::loadCrsb(std::string fileName_noext, const uint32_t subLevel) {
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

    const uint32_t OFFSET_TO_FIRST_CSCN = 0xC; // 12

    // This only gets the first one. The block after this gets them all
    // Address baseAddrCscn = startAddr + OFFSET_TO_FIRST_CSCN;
    // auto mpdzFilename_noext = this->getTextNullTermAt(baseAddrCscn + 0xC);
    // this->loadMpdz(mpdzFilename_noext);

    uint32_t curCscnReadOffset = 0; // In bytes
    uint32_t subLevelIndex = 0;
    for (uint32_t cscnIndex = 0; cscnIndex < mapFileCount; cscnIndex++) {
        // Points to the current magic number text, CSCN
        Address baseAddrCscn = startAddr + OFFSET_TO_FIRST_CSCN + curCscnReadOffset;
        // Check that the magic text is there, at index 0
        std::string magicTextCscn = this->getTextAt(baseAddrCscn + 0, 4);
        if (magicTextCscn.compare(Constants::CSCN_MAGIC) != 0) {
            cerr << "Magic header text " << Constants::CSCN_MAGIC << " not found! Found '" << magicTextCscn << "' instead." << endl;
            return;
        }
        // Next, get the filename
        auto mpdzFilename_noext = this->getTextNullTermAt(baseAddrCscn + 0xC);
        if (subLevelIndex == subLevel) {
            this->loadMpdz(mpdzFilename_noext);
            return;
        }

        // +0x4 is because the magic number is 4 long, and the next is the length
        // +0x8 is because that length is added to the current read position in the file
        //   That recreates it as a non-relative offset number (it started from the
        //   maybeExits at +0x8)
        uint32_t cscnLength = this->getNumberAt<uint32_t>(baseAddrCscn + 0x4) + 0x8;
        
        curCscnReadOffset += cscnLength;
        subLevelIndex++;
    }
}

// TODO: Get rid of this hacky crap
uint32_t timesPaletteLoaded = 0;

void YidsRom::loadMpdz(std::string fileName_noext) {
    if (this->verbose) std::cout << "Loading MPDZ '" << fileName_noext << "'" << endl;
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
        if (this->verbose) std::cout << "[SUCCESS] MPDZ Magic number found" << endl;
    }
    // 4 because the file length is written at bytes 4-7
    uint32_t mpdzFileLength = YUtils::getUint32FromVec(uncompVec, 4);
    // 8 in order to start it at the first instruction besides SET
    Address mpdzIndex = 8; // Pass this in as a pointer to functions

    // TODO: Get rid of this hacky crap
    timesPaletteLoaded = 0;

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
    // cout << "All instructions for '" << mpdzFileName << "' completed" << endl;
}



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
            std::cout << ">> Handling INFO instruction: ";
            uint32_t infoLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First time: 0x20

            uint32_t canvasDimensions = YUtils::getUint32FromVec(mpdzVec, indexPointer + 8); // 00b60208

            // TODO: What are these values?
            uint32_t unknownValue1 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 12); // 00000000
            std::cout << "unk1: " << hex << unknownValue1 << "; ";
            uint32_t unknownValue2 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 16); // 0x1000
            std::cout << "unk2: " << hex << unknownValue2 << "; ";
            uint32_t unknownValue3 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 20); // 0x1000
            std::cout << "unk3: " << hex << unknownValue3 << "; ";
            //uint32_t unknownValue4 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 24); // 0x00020202
            whichBgToWriteTo = mpdzVec.at(indexPointer + 24 + 0);
            std::cout << "whichBg: " << (int)whichBgToWriteTo << "; ";
            // uint16_t charBaseBlockHardMaybe = mpdzVec.at(indexPointer + 24 + 1);
            // uint16_t thirdByte = mpdzVec.at(indexPointer + 24 + 2);
            // uint16_t screenBaseBlockMaybe = mpdzVec.at(indexPointer + 24 + 3);

            uint32_t unknownValue5 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 28); // 00000000
            std::cout << "unk5: " << hex << unknownValue5 << endl;
            Q_UNUSED(unknownValue1);
            Q_UNUSED(unknownValue2);
            Q_UNUSED(unknownValue3);
            Q_UNUSED(unknownValue5);

            // Only the first one matters for the primary height and width, since BG 2 decides everything
            if (whichBgToWriteTo == 2) {
                this->canvasHeightBg2 = canvasDimensions >> 0x10;
                this->canvasWidthBg2 = canvasDimensions % 0x10000;
            } else if (whichBgToWriteTo == 1) {
                this->canvasHeightBg1 = canvasDimensions >> 0x10;
                this->canvasWidthBg1 = canvasDimensions % 0x10000;
            }

            if (infoLength > 0x18) {
                // Get charfile string
                auto charFileNoExt = YUtils::getNullTermTextFromVec(mpdzVec, indexPointer + 32);
                this->handleImbz(charFileNoExt, whichBgToWriteTo);
            }
            // Increment based on earlier length, +8 is to skip instruction and length
            indexPointer += infoLength + 8;
        } else if (curSubInstruction == Constants::PLTB_MAGIC_NUM) {
            uint32_t pltbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            Address pltbReadIndex = indexPointer + 8; // +8 is to skip instruction and length
            indexPointer += pltbLength + 8; // Skip past, don't do a manual count up
            if (timesPaletteLoaded > 0) {
                std::cout << "[WARN] Only BG palette supported thus far, skipping" << endl;
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
            // Most of this tile placing logic is here: 0201c6dc
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
            // Uncomment to get uncompressed MPBZ
            // YUtils::writeByteVectorToFile(compressedSubArray,"2-4-test.mpbz");
            // YCompression::lzssDecomp("2-4-test.mpbz", true);

            indexPointer += mpbzLength + 8; // Skip ahead main pointer to next

            if (whichBgToWriteTo == 3 || whichBgToWriteTo == 0) {
                std::cout << "[WARN] MPBZ tiles other than BG 1 and 2 not implemented, skipping" << endl;
                continue;
            }
            // Handle uncompressedMpbz data
            const uint32_t uncompressedMpbzTwoByteCount = uncompressedMpbz.size() / 2;
            if (uncompressedMpbzTwoByteCount < 1) {
                std::cerr << "[ERROR] uncompressedMpbzTwoByteCount was 0" << std::endl;
            }

            uint32_t mpbzIndex = 0;

            // 0x0201c700
            if (uncompressedMpbz.at(0) == 0xFF && uncompressedMpbz.at(1) == 0xFF) {
                // NOTE: It is not unlikely that there's a third byte for X offset, but
                //   currently it just seems like Y is only what's used. If you see further
                //   sliding, come back and look at this again

                // Get the next WORD, which should be the Y offset/lines skipped
                uint16_t firstOffsetByte = (uint16_t)uncompressedMpbz.at(2);
                uint16_t secondOffsetByte = (uint16_t)uncompressedMpbz.at(3);
                uint16_t offset = (secondOffsetByte << 8) + firstOffsetByte;
                // Skip drawing the number of lines specified in offset
                if (whichBgToWriteTo == 2) {
                    offset = offset * this->canvasWidthBg2;
                } else if (whichBgToWriteTo == 1) {
                    offset = offset * this->canvasWidthBg1;
                }
                
                if (whichBgToWriteTo == 2) {
                    for (int offsetWriteIndex = 0; offsetWriteIndex < offset; offsetWriteIndex++) {
                        this->preRenderDataBg2.push_back(0x0000);
                    } 
                    mpbzIndex += 3; // 0x0201c714
                } else if (whichBgToWriteTo == 1) {
                    // Potentially collapse me later, could probably be merged into above
                    for (int offsetWriteIndex = 0; offsetWriteIndex < offset; offsetWriteIndex++) {
                        this->preRenderDataBg1.push_back(0x0000);
                    } 
                    mpbzIndex += 3; // 0x0201c714
                } else {
                    std::cout << "[WARN] Writing to unhandled BG " << whichBgToWriteTo << std::endl;
                    break;
                    // Do nothing
                }
            }
            while (mpbzIndex < uncompressedMpbzTwoByteCount) {
                uint32_t trueOffset = mpbzIndex*2;
                uint16_t firstByte = (uint16_t)uncompressedMpbz.at(trueOffset);
                uint16_t secondByte = (uint16_t)uncompressedMpbz.at(trueOffset+1);
                uint16_t curShort = (secondByte << 8) + firstByte;
                curShort += 0x1000; // 0201c730
                if (whichBgToWriteTo == 2) {
                    this->preRenderDataBg2.push_back(curShort);
                } else if (whichBgToWriteTo == 1) {
                    this->preRenderDataBg1.push_back(curShort);
                } else {
                    std::cout << "[WARN] Writing to unhandled BG " << whichBgToWriteTo << std::endl;
                    break;
                    // Do nothing
                }
                mpbzIndex++;
            }
            //std::cout << "Finished writing to preRenderDataBg2, length is " << this->preRenderDataBg2.size() << std::endl;
        } else if (curSubInstruction == Constants::COLZ_MAGIC_NUM) {
            if (collisionTileArray.size() > 0) {
                std::cout << "[ERROR] Attempted to load a second COLZ, only one should ever be loaded" << endl;
                exit(EXIT_FAILURE);
            }
            uint32_t colzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First is 0x0b7c
            // Slice out COLZ data
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + colzLength;
            auto colzCompressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            auto uncompressedColz = YCompression::lzssVectorDecomp(colzCompressedSubArray, false);
            this->collisionTileArray = uncompressedColz; // Copy directly
            //YUtils::appendVector(this->collisionTileArray,uncompressedColz);
            if (whichBgToWriteTo == 2) {
                this->canvasWidthCol = this->canvasWidthBg2;
                this->canvasHeightCol = this->canvasHeightBg2;
            } else if (whichBgToWriteTo == 1) {
                this->canvasWidthCol = this->canvasWidthBg1;
                this->canvasHeightCol = this->canvasHeightBg1;
            } else {
                std::cout << "[WARN] Using collision on unsupported BG: " << hex << whichBgToWriteTo << endl;
            }
            indexPointer += colzLength + 8;
        } else if (curSubInstruction == Constants::ANMZ_MAGIC_NUM) {
            // TODO: Figure out how this knows where to write. Take a break, so far you have most tiles loading
            //std::cout << ">> Handling ANMZ instruction" << endl;
            uint32_t anmzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // Should be 0x1080 first time
            // Address compressedDataStart = indexPointer + 8;
            // Address compressedDataEnd = compressedDataStart + anmzLength;
            // auto compressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            // auto uncompressedAnmz = YCompression::lzssVectorDecomp(compressedSubArray, false);

            // // Uncomment to get decompressed ANMZ
            // // YUtils::writeByteVectorToFile(uncompressedAnmz,"test.anmz");
            // // bool decompResult = YCompression::lzssDecomp("test.anmz", verbose);

            // // TODO: Do something with this data
            // const uint32_t ANMZ_INCREMENT = 0x20;
            // const uint32_t ANMZ_HEADER_BASE_LENGTH = 0x8;
            // auto animationFrameCount = uncompressedAnmz.at(0);
            // //cout << "Animation Frame Count: " << dec << (int)animationFrameCount << endl;
            // uint32_t anmzFileIndex = ANMZ_HEADER_BASE_LENGTH + animationFrameCount;
            // uint32_t anmzDataLength = ANMZ_INCREMENT * 0x20; // Pull first 32 for example
            // uint32_t anmzDataIndexEnd = anmzFileIndex + anmzDataLength;
            // uint32_t currentTileIndex = this->pixelTilesBg2.size(); // Size is last index + 1 already
            // while(anmzFileIndex < anmzDataIndexEnd) {
            //     Chartile curTile;
            //     curTile.engine = ScreenEngine::A;
            //     curTile.index = currentTileIndex;
            //     curTile.tiles.resize(64);
            //     // Go up by 2 since you split the bytes
            //     for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
            //         uint8_t curByte = uncompressedAnmz.at(anmzFileIndex + currentTileBuildIndex);
            //         uint8_t highBit = curByte >> 4;
            //         uint8_t lowBit = curByte % 0x10;
            //         int innerPosition = currentTileBuildIndex*2;
            //         curTile.tiles[innerPosition+1] = highBit;
            //         curTile.tiles[innerPosition+0] = lowBit;
            //     }
            //     if (whichBgToWriteTo == 2) {
            //         this->pixelTilesBg2.push_back(curTile);
            //     } else if (whichBgToWriteTo == 1) {
            //         this->pixelTilesBg1.push_back(curTile);
            //     } else {
            //         std::cout << "[WARN] Writing ANMZ to unhandled BG: " << whichBgToWriteTo << endl;
            //     }
            //     // for (int i = 0; i < 64; i++) {
            //     //     std::cout << hex << (int)curTile.tiles[i] << ",";
            //     // }
            //     // cout << endl;
                
            //     // Skip ahead by 0x20
            //     anmzFileIndex += Constants::CHARTILE_DATA_SIZE;
            //     currentTileIndex++;
            // }
            // cout << "Wrote ANMZ to bg: " << whichBgToWriteTo << endl;

            indexPointer += anmzLength + 8; // Go to next
        } else if (curSubInstruction == Constants::IMGB_MAGIC_NUM) {
            //std::cout << ">> Handling IMGB instruction" << endl;
            uint32_t imgbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            // TODO: Learn about this data
            indexPointer += imgbLength + 8;
        } else if (curSubInstruction == Constants::SCEN_MAGIC_NUM) {
            cerr << "[ERROR] Found SCEN instruction, overflowed!" << endl;
            return;
        } else {
            std::cout << "Unknown instruction: " << hex << curSubInstruction << endl;
            return;
        }
    }
}

void YidsRom::handleImbz(std::string fileName_noext, uint16_t whichBg) {
    //if (this->verbose) std::cout << ">> Handling IMBZ file: '" << fileName_noext << "'" << endl;
    if (whichBg == 2) {
        if (this->pixelTilesBg2.size() > 0) {
            cerr << "[ERROR] No overwriting existing pixel tile data for BG 2!" << endl;
            return;
        }
    } else if (whichBg == 1) {
        if (this->pixelTilesBg1.size() > 0) {
            cerr << "[ERROR] No overwriting existing pixel tile data for BG 1!" << endl;
            return;
        }
    } else {
        cerr << "[WARN] Attempting to write to unhandled pixel tiles for BG " << whichBg << endl;
        return;
    }

    auto uncompressedFileVector = this->getFileByteVector(fileName_noext.append(".imbz"));
    std::vector uncompressedImbz = YCompression::lzssVectorDecomp(uncompressedFileVector,false);
    uncompressedFileVector.clear();

    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles
    int imbzIndex = 0; // Goes up by 0x20/32 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    if (imbzLength < 1) {
        std::cerr << "[ERROR] imbzLength is 0!" << std::endl;
        return;
    }
    // Do it 0x20 by 0x20 (32)
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
        if (whichBg == 2) {
            this->pixelTilesBg2.push_back(curTile);
        } else if (whichBg == 1) {
            this->pixelTilesBg1.push_back(curTile);
        }
        
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
    std::cout << "*** Starting GRAD instruction parse ***" << endl;
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
    indexPointer += 4; // Go to length
    auto setdLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    // Now at start of actual data
    indexPointer += 4;
    const uint32_t indexEnd = indexPointer + setdLength;
    while (indexPointer < indexEnd) {
        LevelObject lo;
        lo.objectId = YUtils::getUint16FromVec(mpdzVec, indexPointer + 0);
        lo.settingsLength = YUtils::getUint16FromVec(mpdzVec, indexPointer + 2);
        uint16_t len = lo.settingsLength;
        if (len > 0x20) {
            std::cout << "[WARN] Unusually high object settings length for " << hex << lo.objectId << ": " << hex << len << endl;
        }
        lo.xPosition = YUtils::getUint16FromVec(mpdzVec, indexPointer + 4);
        lo.yPosition = YUtils::getUint16FromVec(mpdzVec, indexPointer + 6);
        
        indexPointer += 8; // This skips to either the settings, or the next object
        if (len > 0) {
            lo.settings = YUtils::subVector(mpdzVec,indexPointer,indexPointer + len);
            indexPointer += len;
        }
        this->loadedLevelObjects.push_back(lo);
    }
}

void YidsRom::handleObjPltFile(std::string objset_filename, std::map<uint32_t,std::vector<uint8_t>>& pixelTiles, std::map<uint32_t,ObjectPalette>& palettes) {
    std::cout << "handleObjPltFile: " << objset_filename << endl;
    std::vector<uint8_t> fileVectorObjset = this->getFileByteVector(objset_filename);
    std::vector<uint8_t> objsetUncompressedVec = YCompression::lzssVectorDecomp(fileVectorObjset,false);

    auto potentialMagicNumber = YUtils::getUint32FromVec(objsetUncompressedVec,0);
    if (potentialMagicNumber != Constants::OBAR_MAGIC_NUM) {
        cerr << "[ERROR] OBAR magic number not found in file '" << objset_filename << "'! Found instead: " << hex << potentialMagicNumber << endl;
        exit(EXIT_FAILURE);
    }
    auto fullObjsetLength = YUtils::getUint32FromVec(objsetUncompressedVec,4);
    //std::cout << "Full length: " << hex << fullObjsetLength << endl;
    uint32_t indexObjset = 8; // magic number (4) + length uint32 (4)
    //std::cout << "First index instruction: " << hex << (int)objsetUncompressedVec.at(indexObjset) << endl;
    const uint32_t objsetEndIndex = fullObjsetLength + 8; // Exclusive, but shouldn't matter
    //std::cout << "End index: " << hex << objsetEndIndex << endl;
    uint32_t currentPaletteIndex = 0;
    uint32_t curTileStartOffset = 0;
    while (indexObjset < objsetEndIndex) {
        auto instructionCheck = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);
        //std::cout << "instructionCheck: " << hex << instructionCheck << endl;
        indexObjset += 4; // Skip instruction, go to length
        auto currentInstructionLength = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);
        indexObjset += 4; // Skip length, go to first
        //std::cout << "Found sublength: " << hex << currentInstructionLength << endl;
        auto subsection = YUtils::subVector(objsetUncompressedVec,indexObjset,indexObjset + currentInstructionLength);
        if (instructionCheck == Constants::OBJB_MAGIC_NUM) {
            /************
             *** OBJB ***
             ************/
            pixelTiles[curTileStartOffset] = subsection;
        } else if (instructionCheck == Constants::PLTB_MAGIC_NUM) {
            /************
             *** PLTB ***
             ************/
            uint32_t subSectionSize = subsection.size();
            if (subSectionSize != Constants::PALETTE_SIZE) {
                cerr << "[WARN] PLTB data not 0x20/32 bytes! Was instead: " << hex << subSectionSize;
                cerr << ". Only pulling first one." << endl;
            }
            ObjectPalette currentLoadingPalette;
            currentLoadingPalette.paletteData.resize(Constants::PALETTE_SIZE);
            for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
                currentLoadingPalette.paletteData[curPaletteIndex] = subsection.at(curPaletteIndex);
            }
            currentLoadingPalette.index = currentPaletteIndex;
            currentPaletteIndex++;
            currentLoadingPalette.address = indexObjset;
            // Does not start at zero! Access is offset by 
            palettes[curTileStartOffset] = currentLoadingPalette;
        } else {
            std::cerr << "[ERROR] Known objset magic number not found! Instead found ";
            std::cerr << hex << instructionCheck << " at " << hex << (indexObjset - 4) << std::endl;
            exit(EXIT_FAILURE);
        }
        curTileStartOffset++;
        indexObjset += currentInstructionLength;
    }
    std::cout << "Loaded " << dec << pixelTiles.size() << " object tile groups" << std::endl;
    std::cout << "Loaded " << dec << palettes.size() << " palettes" << std::endl;
}