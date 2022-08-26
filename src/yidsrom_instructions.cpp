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

        uint32_t trueDataStart = crsbIndex + 8;
        curCscnData.numMapEnters = this->getNumberAt<uint16_t>(crsbIndex + 8);
        curCscnData.numExitsInScene = this->getNumberAt<uint8_t>(crsbIndex + 10);
        // Is 1 bytem but Enums are 4 bytes
        curCscnData.musicId = (CscnMusicId)this->getNumberAt<uint8_t>(crsbIndex + 11);
        auto mpdzText = this->getTextNullTermAt(crsbIndex + 12);
        curCscnData.mpdzFileNoExtension = mpdzText;

        // Yes, that's a hard coded 0x14
        uint32_t postStringIndex = (trueDataStart + 0x14); // 02033224

        uint32_t entrancesIndex = 0;
        uint32_t entrancesAddress = postStringIndex;
        while (entrancesIndex < curCscnData.numMapEnters) {
            auto xEntry = this->getNumberAt<uint16_t>(entrancesAddress+0);
            auto yEntry = this->getNumberAt<uint16_t>(entrancesAddress+2);
            auto returnAnimAndScreen = this->getNumberAt<uint16_t>(entrancesAddress+4);
            
            CscnEnterIntoMap curRet;
            curRet.entranceX = xEntry;
            curRet.entranceY = yEntry;

            // NOTE: Those 0x8000 ones: do >> 14 and check if its equal to 2 (0b10)
            //   if it is 2, Yoshi starts on the bottom screen. Anything else, he starts on the top
            //   This is why there are so many that start with 0x8nnn
            curRet.screen = returnAnimAndScreen >> 14;
            curRet.enterMapAnimation = (ExitAnimation)(returnAnimAndScreen % 0x1000);

            curCscnData.entrances.push_back(curRet);

            entrancesAddress += 6; // Length of total is 6 bytes
            entrancesIndex++;
        }

        // The following instructions are done at 02033238-02033240
        // Unsure where 3 and FF..FC is from, but its hard coded so can't do much
        uint16_t finalExitsOffset = ((6 * curCscnData.numMapEnters) + 3) & 0xFFFFFFFC;
        postStringIndex += finalExitsOffset;
        
        uint32_t exitsIndex = 0;
        while (exitsIndex < curCscnData.numExitsInScene) {
            auto exitTargetX = this->getNumberAt<uint16_t>(postStringIndex+0);
            auto exitTargetY = this->getNumberAt<uint16_t>(postStringIndex+2);
            auto exitStartType = this->getNumberAt<uint16_t>(postStringIndex+4);
            auto whichMap = this->getNumberAt<uint8_t>(postStringIndex+6);
            auto whichEntrance = this->getNumberAt<uint8_t>(postStringIndex+7);

            CscnExitData curExit;
            curExit.exitLocationX = exitTargetX;
            curExit.exitLocationY = exitTargetY;
            curExit.exitStartType = (ExitStartType)exitStartType;
            curExit.whichMapTo = whichMap;
            curExit.whichEntranceTo = whichEntrance;

            curCscnData.exits.push_back(curExit);

            postStringIndex += 8; // It's 8 bytes
            exitsIndex++;
        }

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
    MapFile mf;
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
ScenData YidsRom::handleSCEN(std::vector<uint8_t>& mpdzVec, Address& indexPointer) {
    ScenData scenData;
    
    uint16_t whichBgToWriteTo = 0;
    uint32_t whichBgColorModeMaybe = 0; // 0 = 16, 1 = 256
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    if (instructionCheck != Constants::SCEN_MAGIC_NUM) {
        YUtils::printDebug("SCEN instruction did not find magic hex",DebugType::ERROR);
        return scenData;
    }
    indexPointer += sizeof(uint32_t);
    uint32_t scenLength = YUtils::getUint32FromVec(mpdzVec, indexPointer);
    indexPointer += sizeof(uint32_t);
    const uint32_t scenCutoff = indexPointer + scenLength;
    while (indexPointer < scenCutoff) {
        uint32_t curSubInstruction = YUtils::getUint32FromVec(mpdzVec,indexPointer);
        if (curSubInstruction == Constants::INFO_MAGIC_NUM) {
            InfoData infoData;

            uint32_t infoLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First time: 0x20
            whichBgToWriteTo = mpdzVec.at(indexPointer + 24);
            uint32_t canvasDimensions = YUtils::getUint32FromVec(mpdzVec, indexPointer + 8); // 00b60208
            infoData.layerHeight = canvasDimensions >> 0x10;
            infoData.layerWidth = canvasDimensions % 0x10000;
            infoData.whichBg = whichBgToWriteTo;
            infoData.bgYOffsetMaybe = YUtils::getUint32FromVec(mpdzVec, indexPointer + 12);
            infoData.xOffset = YUtils::getUint32FromVec(mpdzVec, indexPointer + 16);
            infoData.yOffset = YUtils::getUint32FromVec(mpdzVec, indexPointer + 20);
            infoData.layerOrderMaybe = mpdzVec.at(indexPointer + 24 + 1);
            infoData.unkThirdByte = mpdzVec.at(indexPointer + 24 + 2);
            infoData.screenBaseBlockMaybe = mpdzVec.at(indexPointer + 24 + 3);
            infoData.colorModeMaybe = mpdzVec.at(indexPointer + 28);
            whichBgColorModeMaybe = infoData.colorModeMaybe;

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
                auto gotImbz = this->handleImbz(charFileNoExt, whichBgToWriteTo);
                infoData.tileGraphics = gotImbz;
            } else {
                infoData.tileGraphics.fileName = "none";
            }

            scenData.minorInstructions.push_back(&infoData);
            // Increment based on earlier length, +8 is to skip instruction and length
            indexPointer += infoLength + 8;
        } else if (curSubInstruction == Constants::PLTB_MAGIC_NUM) {
            PltbData pltbData;
            uint32_t pltbLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            Address pltbReadIndex = indexPointer + 8; // +8 is to skip instruction and length
            
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
                pltbData.palettes.push_back(currentLoadingPalette);
                pltbReadIndex += Constants::PALETTE_SIZE; // 1 palette is 32 bytes, or 0x20
            }
            scenData.minorInstructions.push_back(&pltbData);
            timesPaletteLoaded++;
        } else if (curSubInstruction == Constants::MPBZ_MAGIC_NUM) {
            // Most of this tile placing logic is here: 0201c6dc
            if (whichBgToWriteTo == 0) {
                YUtils::printDebug("Which BG to write to was not specified, MPBZ load failed",DebugType::ERROR);
                return scenData;
            }
            uint32_t mpbzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4);
            // Slice out MPBZ data
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + mpbzLength;
            auto compressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            // Decompress MPBZ data
            auto uncompressedMpbz = YCompression::lzssVectorDecomp(compressedSubArray, false);

            indexPointer += mpbzLength + 8; // Skip ahead main pointer to next

            MpbzData mpbzData = this->handleMPBZ(uncompressedMpbz,whichBgToWriteTo);
            mpbzData.whichBg = whichBgToWriteTo;
            mpbzData.bgColorMode = whichBgColorModeMaybe;
            scenData.minorInstructions.push_back(&mpbzData);
        } else if (curSubInstruction == Constants::COLZ_MAGIC_NUM) {
            if (collisionTileArray.size() > 0) {
                std::cout << "[ERROR] Attempted to load a second COLZ, only one should ever be loaded" << endl;
                exit(EXIT_FAILURE);
            }
            ColzData colzData;
            uint32_t colzLength = YUtils::getUint32FromVec(mpdzVec, indexPointer + 4); // First is 0x0b7c
            // Slice out COLZ data
            Address compressedDataStart = indexPointer + 8;
            Address compressedDataEnd = compressedDataStart + colzLength;
            auto colzCompressedSubArray = YUtils::subVector(mpdzVec, compressedDataStart, compressedDataEnd);
            auto uncompressedColz = YCompression::lzssVectorDecomp(colzCompressedSubArray, false);
            this->collisionTileArray = uncompressedColz; // Copy directly
            colzData.colArray = uncompressedColz;
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

            scenData.minorInstructions.push_back(&colzData);
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
            // TODO: Learn about this data (020202a4)
            uint32_t scrlDataStart = indexPointer + 8;
            uint32_t scrlDataEnd = scrlDataStart + scrlLength;
            auto dataVec = YUtils::subVector(mpdzVec,scrlDataStart,scrlDataEnd);
            Q_UNUSED(dataVec);
            //YUtils::printVector(dataVec);
            indexPointer += scrlLength + 8;
        } else if (curSubInstruction == Constants::SCEN_MAGIC_NUM) {
            YUtils::printDebug("Found SCEN instruction, overflowed!",DebugType::ERROR);
            return scenData;
        } else {
            std::cout << "Unknown inter-SCEN instruction: " << hex << curSubInstruction << endl;
            return scenData;
        }
    }
    return scenData;
}

ImbzData YidsRom::handleImbz(std::string fileName_noext, uint16_t whichBg) {
    ImbzData imbzData;
    imbzData.fileName = fileName_noext;
    imbzData.whichBg = whichBg;

    this->pixelTilesBg1index = 0;
    this->pixelTilesBg2index = 0;

    auto compressedFileVector = this->getFileByteVector(fileName_noext.append(".imbz"));
    std::vector uncompressedImbz = YCompression::lzssVectorDecomp(compressedFileVector,false);
    compressedFileVector.clear();

    // Use ints since they're natural and not stored excessively anyway
    int currentTileIndex = 0; // The index of the tile within list of tiles
    int imbzIndex = 0; // Goes up by 0x20/32 each time, offset it
    const int imbzLength = uncompressedImbz.size();
    if (imbzLength < 1) {
        YUtils::printDebug("imbzLength is 0!",DebugType::ERROR);
        return imbzData;
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

        imbzData.pixelTiles.push_back(curTile);
        
        // Skip ahead by 0x20
        imbzIndex += Constants::CHARTILE_DATA_SIZE;
        currentTileIndex++;
    }

    // cout << "lol1" << endl;
    // YUtils::printVector(compressedFileVector);
    // auto packed = imbzData.compile();
    // cout << "lol2" << endl;
    // YUtils::printVector(packed);
    // exit(EXIT_SUCCESS);

    return imbzData;
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

PathData YidsRom::handlePATH(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    PathData pathData;
    pathData.pathCount = 0;
    if (instructionCheck != Constants::PATH_MAGIC_NUM) {
        YUtils::printDebug("PATH instruction did not find magic hex",DebugType::ERROR);
        return pathData;
    }

    indexPointer += 4; // Go to length
    auto pathLength = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    indexPointer += 4; // Now at start of actual data

    auto pathCount = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    pathData.pathCount = pathCount;

    // +4 to skip the count
    auto pathVec = YUtils::subVector(mpdzVec,indexPointer+4,indexPointer + pathLength);

    for (uint32_t pathBase = 0; pathBase < pathVec.size(); pathBase += 12) {
        PathRecord pathRec;
        pathRec.angle = YUtils::getUint16FromVec(pathVec,pathBase + 0);
        pathRec.distance = YUtils::getInt16FromVec(pathVec,pathBase + 2);
        pathRec.startX = YUtils::getUint32FromVec(pathVec,pathBase + 4);
        pathRec.startY = YUtils::getUint32FromVec(pathVec,pathBase + 8);
        // std::cout << pathRec.toString();
        // printf("%d\n",pathRec.distance);
        Q_UNUSED(pathRec);
    }

    // Do stuff here
    // For now, skip
    indexPointer += pathLength;

    return pathData;
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

SetdObjectData YidsRom::handleSETD(std::vector<uint8_t>& mpdzVec, uint32_t& indexPointer) {
    uint32_t instructionCheck = YUtils::getUint32FromVec(mpdzVec,indexPointer);
    SetdObjectData setdObjectData;
    if (instructionCheck != Constants::SETD_MAGIC_NUM) {
        YUtils::printDebug("SETD instruction did not find magic hex",DebugType::ERROR);
        return setdObjectData;
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
        setdObjectData.levelObjects.push_back(lo);
        this->loadedLevelObjects.push_back(lo);
        //std::cout << lo.toString() << std::endl;
    }
    return setdObjectData;
}

ObjectFile YidsRom::getMajorObjPltFile(std::string objset_filename, std::map<uint32_t,std::vector<uint8_t>>& pixelTiles, std::map<uint32_t,ObjectPalette>& palettes) {
    ObjectFile objFileData;
    objFileData.objectFileName = objset_filename;
    
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

    uint32_t indexObjset = 8; // magic number (4) + length uint32 (4)

    const uint32_t objsetEndIndex = fullObjsetLength + 8; // Exclusive, but shouldn't matter

    uint32_t currentPaletteIndex = 0;
    uint32_t curTileStartOffset = 0;
    while (indexObjset < objsetEndIndex) {
        auto instructionCheck = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);

        indexObjset += 4; // Skip instruction, go to length
        auto currentInstructionLength = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);
        indexObjset += 4; // Skip length, go to first

        auto subsection = YUtils::subVector(objsetUncompressedVec,indexObjset,indexObjset + currentInstructionLength);
        if (instructionCheck == Constants::OBJB_MAGIC_NUM) {
            /************
             *** OBJB ***
             ************/
            pixelTiles[curTileStartOffset] = subsection;
            objFileData.objectPixelTiles[curTileStartOffset] = subsection;
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
            objFileData.objectPalettes[curTileStartOffset] = currentLoadingPalette;
        } else {
            std::cerr << "[ERROR] Known objset magic number not found! Instead found ";
            std::cerr << hex << instructionCheck << " at " << hex << (indexObjset - 4) << std::endl;
            exit(EXIT_FAILURE);
        }
        curTileStartOffset++;
        indexObjset += currentInstructionLength;
    }
    if (objFileData.objectPixelTiles.size() < 1) {
        YUtils::printDebug("Pulled zero OBJB records",DebugType::ERROR);
    }
    if (objFileData.objectPalettes.size() < 1) {
        YUtils::printDebug("Pulled zero PLTB records",DebugType::ERROR);
    }
    return objFileData;
}

ObjectFile YidsRom::getObjPltFile(std::string objset_filename) {
    ObjectFile objFileData;
    objFileData.objectFileName = objset_filename;
    
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

    uint32_t indexObjset = 8; // magic number (4) + length uint32 (4)

    const uint32_t objsetEndIndex = fullObjsetLength + 8; // Exclusive, but shouldn't matter

    uint32_t currentPaletteIndex = 0;
    uint32_t curTileStartOffset = 0;
    while (indexObjset < objsetEndIndex) {
        auto instructionCheck = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);

        indexObjset += 4; // Skip instruction, go to length
        auto currentInstructionLength = YUtils::getUint32FromVec(objsetUncompressedVec,indexObjset);
        indexObjset += 4; // Skip length, go to first

        auto subsection = YUtils::subVector(objsetUncompressedVec,indexObjset,indexObjset + currentInstructionLength);
        if (instructionCheck == Constants::OBJB_MAGIC_NUM || instructionCheck == Constants::OBJZ_MAGIC_NUM) {
            /**************
             *** OBJB/Z ***
             **************/
            if (instructionCheck == Constants::OBJZ_MAGIC_NUM) {
                subsection = YCompression::lzssVectorDecomp(subsection,false);
            }
            objFileData.objectPixelTiles[curTileStartOffset] = subsection;
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
            objFileData.objectPalettes[curTileStartOffset] = currentLoadingPalette;
        } else {
            std::cerr << "[ERROR] Known objset magic number not found! Instead found ";
            std::cerr << hex << instructionCheck << " at " << hex << (indexObjset - 4) << std::endl;
            exit(EXIT_FAILURE);
        }
        curTileStartOffset++;
        indexObjset += currentInstructionLength;
    }
    if (objFileData.objectPixelTiles.size() < 1) {
        YUtils::printDebug("Pulled zero OBJB records",DebugType::ERROR);
    }
    if (objFileData.objectPalettes.size() < 1) {
        YUtils::printDebug("Pulled zero PLTB records",DebugType::ERROR);
    }
    return objFileData;
}

MpbzData YidsRom::handleMPBZ(std::vector<uint8_t>& uncompressedMpbz, uint16_t whichBg) {
    MpbzData mpbzData;
    mpbzData.whichBg = whichBg;

    // Handle uncompressedMpbz data
    const uint32_t uncompressedMpbzTwoByteCount = uncompressedMpbz.size() / 2;
    if (uncompressedMpbzTwoByteCount < 1) {
        YUtils::printDebug("uncompressedMpbzTwoByteCount was 0",DebugType::ERROR);
        return mpbzData;
    }

    uint32_t mpbzIndex = 0;

    // 0x0201c700
    if (uncompressedMpbz.at(0) == 0xFF && uncompressedMpbz.at(1) == 0xFF) {
        // NOTE: It is not unlikely that there's a third byte for X offset, but
        //   currently it just seems like Y is only what's used. If you see further
        //   sliding, come back and look at this again

        // Get the next WORD, which should be the Y offset/lines skipped
        auto offset = YUtils::getUint16FromVec(uncompressedMpbz,2);
        mpbzData.tileOffset = offset;

        auto bottomTrim = YUtils::getUint16FromVec(uncompressedMpbz,4);
        // TODO: Implement?
        mpbzData.bottomTrim = bottomTrim;

        // Skip drawing the number of lines specified in offset
        if (whichBg == 2) {
            offset = offset * this->canvasWidthBg2;
        } else if (whichBg == 1) {
            offset = offset * this->canvasWidthBg1;
        }
        
        if (whichBg == 2) {
            for (int offsetWriteIndex = 0; offsetWriteIndex < offset; offsetWriteIndex++) {
                this->preRenderDataBg2.push_back(0x0000);
            } 
            mpbzIndex += 3; // 0x0201c714
        } else if (whichBg == 1) {
            // Potentially collapse me later, could probably be merged into above
            for (int offsetWriteIndex = 0; offsetWriteIndex < offset; offsetWriteIndex++) {
                this->preRenderDataBg1.push_back(0x0000);
            } 
            mpbzIndex += 3; // 0x0201c714
        } else if (whichBg == 3) {
            // Potentially collapse me later, could probably be merged into above
            for (int offsetWriteIndex = 0; offsetWriteIndex < offset; offsetWriteIndex++) {
                this->preRenderDataBg3.push_back(0x0000);
            } 
            mpbzIndex += 3; // 0x0201c714
        } else {
            std::stringstream ssUnhandledBg;
            ssUnhandledBg << "Writing to unhandled BG " << whichBg;
            YUtils::printDebug(ssUnhandledBg.str(),DebugType::WARNING);
            return mpbzData;
        }
    } else {
        mpbzData.tileOffset = 0;
    }

    while (mpbzIndex < uncompressedMpbzTwoByteCount) {
        uint32_t trueOffset = mpbzIndex*2;
        uint16_t firstByte = (uint16_t)uncompressedMpbz.at(trueOffset);
        uint16_t secondByte = (uint16_t)uncompressedMpbz.at(trueOffset+1);
        uint16_t curShort = (secondByte << 8) + firstByte;
        curShort += 0x1000; // 0201c730
        mpbzData.tileRenderData.push_back(curShort);
        if (whichBg == 2) {
            this->preRenderDataBg2.push_back(curShort);
        } else if (whichBg == 1) {
            this->preRenderDataBg1.push_back(curShort);
        } else if (whichBg == 3) {
            this->preRenderDataBg3.push_back(curShort);
        } else {
            std::stringstream ssUnhandledBg;
            ssUnhandledBg << "Writing to unhandled BG " << whichBg;
            YUtils::printDebug(ssUnhandledBg.str(),DebugType::WARNING);
            break;
        }
        mpbzIndex++;
    }
    return mpbzData;
}