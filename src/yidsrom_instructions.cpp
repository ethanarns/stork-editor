#include "yidsrom.h"
#include "compression.h"
#include "Chartile.h"
#include "utils.h"
#include "constants.h"
#include "LevelObject.h"
#include "PixelDelegate.h"
#include "popups/PaletteTable.h"
#include "Level.h"
#include "FsPacker.h"

#include <iostream>
#include <vector>

using namespace std;

CrsbData YidsRom::loadCrsb(std::string fileName_noext) {
    CrsbData crsbData;

    fileName_noext = YUtils::getLowercase(fileName_noext);
    auto fileName = fileName_noext.append(".crsb");
    auto fileId = this->fileIdMap[fileName];
    if (fileId == 0) {
        cerr << "Failed to load level: " << fileName << endl;
        return crsbData;
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
        return crsbData;
    }
    
    std::string magicText = this->getTextAt(startAddr + 0,4);
    if (magicText.compare(Constants::CRSB_MAGIC) != 0) {
        cerr << "Magic header text " << Constants::CRSB_MAGIC << " not found! Found '" << magicText << "' instead." << endl;
        exit(EXIT_FAILURE);
    }

    auto mapFileCount = this->getNumberAt<uint32_t>(startAddr + 8);
    if (mapFileCount == 0) {
        YUtils::printDebug("mapFileCount in CRSB was 0",DebugType::ERROR);
        return crsbData;
    }
    crsbData.mapFileCount = mapFileCount;

    uint32_t crsbIndex = startAddr + 0x0C; // All important data retrieved, jump to the first CSCN

    uint32_t mapFileIndex = 0;
    while (mapFileIndex < mapFileCount) {
        CscnData curCscnData;

        // Check that the magic text is there, at index 0
        std::string magicTextCscn = this->getTextAt(crsbIndex + 0, 4);
        if (magicTextCscn.compare(Constants::CSCN_MAGIC) != 0) {
            std::stringstream cscnMagic;
            cscnMagic << "Magic header text " << Constants::CSCN_MAGIC << " not found! Found '" << magicTextCscn << "' instead.";
            YUtils::printDebug(cscnMagic.str(),DebugType::ERROR);
            return crsbData;
        }

        uint32_t cscnLength = this->getNumberAt<uint32_t>(crsbIndex + 4);

        // TODO: Figure out why uint16 doesn't work with getNumber at.
        // Maybe this? https://stackoverflow.com/questions/10632251/undefined-reference-to-template-function
        this->romFile.seekg(crsbIndex + 8);
        this->romFile.read(reinterpret_cast<char *>(&curCscnData.exitCount), sizeof(curCscnData.exitCount));

        curCscnData.unknown_16 = this->getNumberAt<uint8_t>(crsbIndex + 10);
        curCscnData.unknown_17 = this->getNumberAt<uint8_t>(crsbIndex + 11);
        auto mpdzText = this->getTextNullTermAt(crsbIndex + 12);
        curCscnData.mpdzFileNoExtension = mpdzText;

        // +1 accounts for the null terminator (doesn't count with size())
        uint32_t postStringIndex = crsbIndex + 12 + curCscnData.mpdzFileNoExtension.size() + 1;
        uint32_t endPostString = crsbIndex + cscnLength + 0x08;

        // Note: There always seems to be 8 zeroes before anything happens post-string
        // It makes no sense, but I have yet to open a level with anything but 8 zeroes
        postStringIndex += 8;

        while (postStringIndex < endPostString) {
            uint16_t curPostStringValue;
            this->romFile.seekg(postStringIndex);
            this->romFile.read(reinterpret_cast<char *>(&curPostStringValue), sizeof(curPostStringValue));
            cout << hex << setw(4) << std::setfill('0') << curPostStringValue << " ";
            postStringIndex += 2;
        }
        cout << endl;

        //cout << "lol " << hex << (int)this->getNumberAt<uint8_t>(postStringIndex-2) << endl;

        // Finally, add it to the parent object
        crsbData.cscnList.push_back(curCscnData);

        // +8 because the length doesn't factor in magic hex and length
        crsbIndex += cscnLength + 0x8;

        mapFileIndex++;
    }

    if (crsbData.cscnList.size() == 0) {
        YUtils::printDebug("CRSB pulled 0 CSCN records",DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    if (crsbData.cscnList.size() != crsbData.mapFileCount) {
        YUtils::printDebug("CRSB mismatch between CSCN record count and mapFileCount",DebugType::FATAL);
        exit(EXIT_FAILURE);
    }

    return crsbData;
}

// TODO: Get rid of this hacky crap
uint32_t timesPaletteLoaded = 0;
int globalPaletteIndex = 1;

void YidsRom::loadMpdz(std::string fileName_noext) {
    std::stringstream ssMpdzLoad;
    ssMpdzLoad << "Loading MPDZ (Map) '" << fileName_noext << "'";
    YUtils::printDebug(ssMpdzLoad.str(),DebugType::VERBOSE);
    std::string mpdzFileName = fileName_noext.append(Constants::MPDZ_EXTENSION);
    auto fileVector = this->getFileByteVector(mpdzFileName);
    // YUtils::writeByteVectorToFile(fileVector,mpdzFileName); // Uncomment to get uncompressed MPDZ
    auto uncompVec = YCompression::lzssVectorDecomp(fileVector,false);
    
    uint32_t magic = YUtils::getUint32FromVec(uncompVec,0);
    if (magic != Constants::MPDZ_MAGIC_NUM) {
        std::stringstream ssNoMagicNum;
        ssNoMagicNum << "MPDZ Magic number not found! Expected " << hex << Constants::MPDZ_MAGIC_NUM;
        ssNoMagicNum << ", got " << hex << magic << " instead.";
        YUtils::printDebug(ssNoMagicNum.str(),DebugType::ERROR);
        return;
    }
    // 4 because the file length is written at bytes 4-7
    uint32_t mpdzFileLength = YUtils::getUint32FromVec(uncompVec, 4);
    // 8 in order to start it at the first instruction besides SET
    Address mpdzIndex = 8; // Pass this in as a pointer to functions

    this->pixelTilesBg1index = 0;

    // TODO: Get rid of this hacky crap
    timesPaletteLoaded = 0;
    globalPaletteIndex = 1;

    // Instruction loop
    while (mpdzIndex < mpdzFileLength) {
        uint32_t curInstruction = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
        if (curInstruction == Constants::SCEN_MAGIC_NUM) {
            this->handleSCEN(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::GRAD_MAGIC_NUM) {
            this->handleGrad(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::SETD_MAGIC_NUM) {
            this->handleSETD(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::AREA_MAGIC_NUM) {
            this->handleAREA(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::PATH_MAGIC_NUM) {
            this->handlePATH(uncompVec,mpdzIndex);
        } else if (curInstruction == Constants::ALPH_MAGIC_NUM) {
            this->handleALPH(uncompVec,mpdzIndex);
        } else {
            std::stringstream ssUnkInstScen;
            ssUnkInstScen << "Instruction besides SCEN used: " << hex << curInstruction;
            YUtils::printDebug(ssUnkInstScen.str(),DebugType::ERROR);
            return;
        }
    }
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
        YUtils::printDebug("SCEN instruction did not find magic hex",DebugType::ERROR);
        return;
    }
    indexPointer += sizeof(uint32_t);
    uint32_t scenLength = YUtils::getUint32FromVec(mpdzVec, indexPointer);
    indexPointer += sizeof(uint32_t);
    const uint32_t scenCutoff = indexPointer + scenLength;
    while (indexPointer < scenCutoff) {
        uint32_t curSubInstruction = YUtils::getUint32FromVec(mpdzVec,indexPointer);
        if (curSubInstruction == Constants::INFO_MAGIC_NUM) {
            std::stringstream ssInfo;
            ssInfo << "Handling INFO instruction: ";
            uint32_t infoLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First time: 0x20

            uint32_t canvasDimensions = YUtils::getUint32FromVec(mpdzVec, indexPointer + 8); // 00b60208

            // TODO: What are these values?
            uint32_t unknownValue1 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 12); // 00000000
            ssInfo << "unk1: " << hex << unknownValue1 << "; ";
            uint32_t unknownValue2 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 16); // 0x1000
            ssInfo << "unk2: " << hex << unknownValue2 << "; ";
            uint32_t unknownValue3 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 20); // 0x1000
            ssInfo << "unk3: " << hex << unknownValue3 << "; ";
            //uint32_t unknownValue4 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 24); // 0x00020202
            whichBgToWriteTo = mpdzVec.at(indexPointer + 24 + 0);
            ssInfo << "whichBg: " << (int)whichBgToWriteTo << "; ";
            // uint16_t charBaseBlockHardMaybe = mpdzVec.at(indexPointer + 24 + 1);
            // uint16_t thirdByte = mpdzVec.at(indexPointer + 24 + 2);
            // uint16_t screenBaseBlockMaybe = mpdzVec.at(indexPointer + 24 + 3);

            uint32_t unknownValue5 = YUtils::getUint32FromVec(mpdzVec, indexPointer + 28); // 00000000
            ssInfo << "unk5: " << hex << unknownValue5;
            YUtils::printDebug(ssInfo.str(),DebugType::VERBOSE);
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
            // auto testSub = YUtils::subVector(mpdzVec,pltbReadIndex,pltbReadIndex+pltbLength);
            // YUtils::printVector(testSub);
            indexPointer += pltbLength + 8; // Skip past, don't do a manual count up

            // BGs have offsets
            if (whichBgToWriteTo == 2) {
                this->paletteOffsetBg2 = globalPaletteIndex-1; // Starts at 1
            } else if (whichBgToWriteTo == 1) {
                this->paletteOffsetBg1 = globalPaletteIndex-1;
            }

            while (pltbReadIndex < indexPointer) {
                QByteArray currentLoadingPalette;
                currentLoadingPalette.resize(Constants::PALETTE_SIZE);
                for (uint32_t curPaletteIndex = 0; curPaletteIndex < Constants::PALETTE_SIZE; curPaletteIndex++) {
                    currentLoadingPalette[curPaletteIndex] = mpdzVec.at(pltbReadIndex + curPaletteIndex);
                }
                if (globalPaletteIndex >= PaletteTable::PALETTE_TABLE_HEIGHT) {
                    YUtils::printDebug("More than 0x20 palettes attempted to be added",DebugType::WARNING);
                    break;
                } else {
                    this->currentPalettes[globalPaletteIndex] = currentLoadingPalette;
                    globalPaletteIndex++;
                }
                pltbReadIndex += Constants::PALETTE_SIZE; // 1 palette is 32 bytes, or 0x20
            }
            timesPaletteLoaded++;
        } else if (curSubInstruction == Constants::MPBZ_MAGIC_NUM) {
            // Most of this tile placing logic is here: 0201c6dc
            if (whichBgToWriteTo == 0) {
                YUtils::printDebug("Which BG to write to was not specified, MPBZ load failed",DebugType::ERROR);
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
                YUtils::printDebug("MPBZ tiles other than BG 1 and 2 not implemented, skipping",DebugType::WARNING);
                continue;
            }
            // Handle uncompressedMpbz data
            const uint32_t uncompressedMpbzTwoByteCount = uncompressedMpbz.size() / 2;
            if (uncompressedMpbzTwoByteCount < 1) {
                YUtils::printDebug("uncompressedMpbzTwoByteCount was 0",DebugType::ERROR);
                continue;
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
                    std::stringstream ssUnhandledBg;
                    ssUnhandledBg << "Writing to unhandled BG " << whichBgToWriteTo;
                    YUtils::printDebug(ssUnhandledBg.str(),DebugType::WARNING);
                    break;
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
                    std::stringstream ssUnhandledBg;
                    ssUnhandledBg << "Writing to unhandled BG " << whichBgToWriteTo;
                    YUtils::printDebug(ssUnhandledBg.str(),DebugType::WARNING);
                    break;
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
                std::stringstream ssColErr;
                ssColErr << "Using collision on unsupported BG: " << hex << whichBgToWriteTo;
                YUtils::printDebug(ssColErr.str(),DebugType::WARNING);
            }
            indexPointer += colzLength + 8;
        } else if (curSubInstruction == Constants::ANMZ_MAGIC_NUM) {
            //std::cout << ">> Handling ANMZ instruction" << endl;
            uint32_t anmzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // Should be 0x1080 first time
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + anmzLength;
            auto compressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            auto uncompressedAnmz = YCompression::lzssVectorDecomp(compressedSubArray, false);

            // Uncomment to get decompressed ANMZ
            // YUtils::writeByteVectorToFile(uncompressedAnmz,"test2.anmz");
            // bool decompResult = YCompression::lzssDecomp("test2.anmz");
            // exit(EXIT_SUCCESS);

            //const uint32_t ANMZ_INCREMENT = 0x20;
            const uint32_t ANMZ_HEADER_BASE_LENGTH = 0x8;
            auto animationFrameCount = uncompressedAnmz.at(0);
            //cout << "Animation Frame Count: " << dec << (int)animationFrameCount << endl;
            uint32_t anmzFileIndex = ANMZ_HEADER_BASE_LENGTH + animationFrameCount;
            anmzFileIndex = 0xC;
            uint32_t currentTileIndex = 0;
            uint32_t anmzSize = uncompressedAnmz.size();
            auto startIndex = YUtils::getUint16FromVec(uncompressedAnmz,4);

            if (whichBgToWriteTo == 1) {
                this->pixelTilesBg1index = startIndex;
                currentTileIndex = this->pixelTilesBg1.size(); // Size is last index + 1 already
            } else if (whichBgToWriteTo == 2) {
                this->pixelTilesBg2index = startIndex;
                currentTileIndex = this->pixelTilesBg2.size();
            } else {
                YUtils::printDebug("Unhandled BG in ANMZ",DebugType::WARNING);
            }
            
            while(anmzFileIndex < anmzSize) {
                Chartile curTile;
                curTile.engine = ScreenEngine::A;
                curTile.index = currentTileIndex;
                curTile.tiles.resize(64);
                // Go up by 2 since you split the bytes
                for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
                    uint8_t curByte = uncompressedAnmz.at(anmzFileIndex + currentTileBuildIndex);
                    uint8_t highBit = curByte >> 4;
                    uint8_t lowBit = curByte % 0x10;
                    int innerPosition = currentTileBuildIndex*2;
                    curTile.tiles[innerPosition+1] = highBit;
                    curTile.tiles[innerPosition+0] = lowBit;
                }
                if (whichBgToWriteTo == 2) {
                    this->pixelTilesBg2[this->pixelTilesBg2index++] = curTile;
                    //this->pixelTilesBg2.push_back(curTile);
                } else if (whichBgToWriteTo == 1) {
                    this->pixelTilesBg1[this->pixelTilesBg1index++] = curTile;
                } else {
                    // warn
                }
                
                // Skip ahead by 0x20
                anmzFileIndex += Constants::CHARTILE_DATA_SIZE;
                currentTileIndex++;
            }
            // Reset the start to 0
            if (whichBgToWriteTo == 1) {
                this->pixelTilesBg1index = 0;
            } else if (whichBgToWriteTo == 2) {
                this->pixelTilesBg2index = 0;
            }

            indexPointer += anmzLength + 8; // Go to next
        } else if (curSubInstruction == Constants::IMGB_MAGIC_NUM) {
            //std::cout << ">> Handling IMGB instruction" << endl;
            uint32_t imgbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            const uint32_t imgbEnd = indexPointer + 8 + imgbLength;
            indexPointer += 8;

            // Do it 0x20 by 0x20 (32)
            while (indexPointer < imgbEnd) {
                Chartile curTile;
                curTile.engine = ScreenEngine::A;
                curTile.index = indexPointer; // change this
                curTile.tiles.resize(64);
                // Go up by 2 since you split the bytes
                for (int currentTileBuildIndex = 0; currentTileBuildIndex < Constants::CHARTILE_DATA_SIZE; currentTileBuildIndex++) {
                    uint8_t curByte = mpdzVec.at(indexPointer + currentTileBuildIndex);
                    uint8_t highBit = curByte >> 4;
                    uint8_t lowBit = curByte % 0x10;
                    int innerPosition = currentTileBuildIndex*2;
                    curTile.tiles[innerPosition+1] = highBit;
                    curTile.tiles[innerPosition+0] = lowBit;
                }
                if (whichBgToWriteTo == 2) {
                    this->pixelTilesBg2[this->pixelTilesBg2index++] = curTile;
                } else if (whichBgToWriteTo == 1) {
                    this->pixelTilesBg1[this->pixelTilesBg1index++] = curTile;
                }
                
                // Skip ahead by 0x20
                indexPointer += Constants::CHARTILE_DATA_SIZE;
            }
        } else if (curSubInstruction == Constants::SCRL_MAGIC_NUM) {
            uint32_t scrlLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            // TODO: Learn about this data
            indexPointer += scrlLength + 8;
        } else if (curSubInstruction == Constants::SCEN_MAGIC_NUM) {
            YUtils::printDebug("Found SCEN instruction, overflowed!",DebugType::ERROR);
            return;
        } else {
            std::cout << "Unknown inter-SCEN instruction: " << hex << curSubInstruction << endl;
            return;
        }
    }
}

void YidsRom::handleImbz(std::string fileName_noext, uint16_t whichBg) {
    this->pixelTilesBg1index = 0;
    this->pixelTilesBg2index = 0;

    auto uncompressedFileVector = this->getFileByteVector(fileName_noext.append(".imbz"));
    std::vector uncompressedImbz = YCompression::lzssVectorDecomp(uncompressedFileVector,false);
    uncompressedFileVector.clear();

    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles
    int imbzIndex = 0; // Goes up by 0x20/32 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    if (imbzLength < 1) {
        YUtils::printDebug("imbzLength is 0!",DebugType::ERROR);
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
            this->pixelTilesBg2[this->pixelTilesBg2index++] = curTile;
        } else if (whichBg == 1) {
            this->pixelTilesBg1[this->pixelTilesBg1index++] = curTile;
        }
        
        // Skip ahead by 0x20
        imbzIndex += Constants::CHARTILE_DATA_SIZE;
        currentTileIndex++;
    }
}

void YidsRom::handleGrad(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::GRAD_MAGIC_NUM) {
        YUtils::printDebug("GRAD instruction did not find magic hex",DebugType::ERROR);
        return;
    }
    //std::cout << "*** Starting GRAD instruction parse ***" << endl;
    indexPointer += 4; // Go to length
    auto gradLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    indexPointer += 4; // Now at start of actual data

    // Do stuff here
    // For now, skip
    indexPointer += gradLength;
}

void YidsRom::handleAREA(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::AREA_MAGIC_NUM) {
        YUtils::printDebug("AREA instruction did not find magic hex",DebugType::ERROR);
        return;
    }
    //std::cout << "*** Starting AREA instruction parse ***" << endl;
    indexPointer += 4; // Go to length
    auto areaLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    indexPointer += 4; // Now at start of actual data

    // Do stuff here
    // For now, skip
    indexPointer += areaLength;
}

void YidsRom::handlePATH(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::PATH_MAGIC_NUM) {
        YUtils::printDebug("PATH instruction did not find magic hex",DebugType::ERROR);
        return;
    }
    //std::cout << "*** Starting AREA instruction parse ***" << endl;
    indexPointer += 4; // Go to length
    auto pathLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    indexPointer += 4; // Now at start of actual data

    // Do stuff here
    // For now, skip
    indexPointer += pathLength;
}

void YidsRom::handleALPH(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::ALPH_MAGIC_NUM) {
        YUtils::printDebug("ALPH instruction did not find its magic hex",DebugType::ERROR);
        return;
    }
    //std::cout << "*** Starting AREA instruction parse ***" << endl;
    indexPointer += 4; // Go to length
    auto alphLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    indexPointer += 4; // Now at start of actual data

    // Do stuff here
    // For now, skip
    indexPointer += alphLength;
}

void YidsRom::handleSETD(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::SETD_MAGIC_NUM) {
        YUtils::printDebug("SETD instruction did not find magic hex",DebugType::ERROR);
        return;
    }
    indexPointer += 4; // Go to length
    auto setdLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    // Now at start of actual data
    indexPointer += 4;
    const uint32_t indexEnd = indexPointer + setdLength;
    while (indexPointer < indexEnd) {
        LevelObject lo;
        lo.uuid = this->levelObjectLoadIndex++;
        lo.objectId = YUtils::getUint16FromVec(mpdzVec, indexPointer + 0);
        lo.settingsLength = YUtils::getUint16FromVec(mpdzVec, indexPointer + 2);
        uint16_t len = lo.settingsLength;
        if (len > 0x20) {
            std::stringstream bigObj;
            bigObj << "Unusually high object settings length for " << hex << lo.objectId << ": " << hex << len;
            YUtils::printDebug(bigObj.str(),DebugType::WARNING);
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
    std::stringstream ssGraphics;
    ssGraphics << "Loading graphics archive '" << objset_filename << "'";
    YUtils::printDebug(ssGraphics.str(),DebugType::VERBOSE);
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
                std::stringstream ssPltbSize;
                ssPltbSize << "PLTB data not 0x20/32 bytes! Was instead: " << hex << subSectionSize;
                ssPltbSize << ". Only pulling first one.";
                YUtils::printDebug(ssPltbSize.str(),DebugType::WARNING);
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
    if (pixelTiles.size() < 1) {
        std::cerr << "[ERROR] Pulled zero pixelTiles!" << std::endl;
    }
    if (palettes.size() < 1) {
        std::cerr << "[ERROR] Pulled zero PLTB records!" << std::endl;
    }
    //std::cout << "Loaded " << dec << pixelTiles.size() << " object tile groups" << std::endl;
    //std::cout << "Loaded " << dec << palettes.size() << " palettes" << std::endl;
}