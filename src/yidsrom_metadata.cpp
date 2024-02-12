#include "yidsrom.h"
#include "compression.h"
#include "Chartile.h"
#include "utils.h"
#include "constants.h"
#include "LevelObject.h"

#include <iostream>
#include <vector>

using namespace std;

void YidsRom::getGameLevelsMetaData() {
    cout << "*** BEGIN METADATA DEBUG ***" << endl;
    for (int worldIndex = 0; worldIndex < 5; worldIndex++) {
        for (int levelIndex = 0; levelIndex < 10; levelIndex++) {
            cout << "Loading world index " << worldIndex << ", level index " << levelIndex << ", CRSB '";
            auto curLevelCrsbName = this->getLevelFileNameFromMapIndex(worldIndex,levelIndex);
            cout << curLevelCrsbName << "', map count ";
            /**
             * CRSB LOADING
             */
            curLevelCrsbName = YUtils::getLowercase(curLevelCrsbName);
            auto fileName = curLevelCrsbName.append(".crsb");
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
            cout << mapFileCount << endl;

            const uint32_t OFFSET_TO_FIRST_CSCN = 0xC; // 12

            uint32_t curCscnReadOffset = 0; // In bytes
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
                cout << "  Loading MPDZ (Map) " << mpdzFilename_noext << endl;

                std::string mpdzFileName = mpdzFilename_noext.append(Constants::MPDZ_EXTENSION);
                auto fileVector = this->getByteVectorFromFile(mpdzFileName);
                // YUtils::writeByteVectorToFile(fileVector,mpdzFileName); // Uncomment to get uncompressed MPDZ
                auto uncompVec = YCompression::lzssVectorDecomp(fileVector,false);
                
                uint32_t magic = YUtils::getUint32FromVec(uncompVec,0);
                if (magic != Constants::MPDZ_MAGIC_NUM) {
                    cerr << "MPDZ Magic number not found! Expected " << hex << Constants::MPDZ_MAGIC_NUM;
                    cerr << ", got " << hex << magic << " instead." << endl;
                    return;
                }
                // 4 because the file length is written at bytes 4-7
                uint32_t mpdzFileLength = YUtils::getUint32FromVec(uncompVec, 4);
                // 8 in order to start it at the first instruction besides SET
                Address mpdzIndex = 8; // Pass this in as a pointer to functions

                // Instruction loop
                while (mpdzIndex < mpdzFileLength) {
                    /**
                     * GO INSIDE SCEN
                     */
                    uint32_t curInstruction = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                    if (curInstruction == Constants::SCEN_MAGIC_NUM) {
                        cout << "    SCEN" << endl;
                        uint32_t instructionCheck = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        if (instructionCheck != Constants::SCEN_MAGIC_NUM) {
                            cerr << "SCEN instruction did not find magic hex " << hex << Constants::SCEN_MAGIC_NUM << endl;
                            return;
                        }
                        mpdzIndex += sizeof(uint32_t);
                        uint32_t scenLength = YUtils::getUint32FromVec(uncompVec, mpdzIndex);
                        mpdzIndex += sizeof(uint32_t);
                        const uint32_t scenCutoff = mpdzIndex + scenLength;
                        while (mpdzIndex < scenCutoff) {
                            uint32_t curSubInstruction = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                            cout << "      "; // No endl, this is just padding
                            switch(curSubInstruction) {
                                case Constants::ANMZ_MAGIC_NUM: {
                                    cout << "ANMZ" << endl;
                                    break;
                                }
                                case Constants::IMGB_MAGIC_NUM: {
                                    cout << "IMGB" << endl;
                                    break;
                                }
                                case Constants::INFO_MAGIC_NUM: {
                                    cout << "INFO" << endl;
                                    /**
                                     * INFO
                                     */
                                    cout << "        ";
                                    uint32_t canvasDimensions = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 8); // 00b60208
                                    // Only the first one matters for the primary height and width, since BG 2 decides everything
                                    cout << "canvasHeightBg2: " << (canvasDimensions >> 0x10);
                                    cout << "; canvasWidthBg2: " << canvasDimensions % 0x10000 << endl;

                                    // TODO: What are these values?
                                    uint32_t unknownValue1 = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 12); // 00000000
                                    cout << "        unk1: " << hex << unknownValue1 << "; ";
                                    uint32_t unknownValue2 = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 16); // 0x1000
                                    cout << "unk2: " << hex << unknownValue2 << endl;
                                    uint32_t unknownValue3 = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 20); // 0x1000
                                    cout << "        unk3: " << hex << unknownValue3 << "; ";
                                    //uint32_t unknownValue4 = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 24); // 0x00020202
                                    auto whichBgToWriteTo = uncompVec.at(mpdzIndex + 24 + 0);
                                    cout << "whichBg: " << (int)whichBgToWriteTo << endl;
                                    uint16_t charBaseBlockHardMaybe = uncompVec.at(mpdzIndex + 24 + 1);
                                    cout << "        charBaseBlockHardMaybe: " << (int)charBaseBlockHardMaybe << "; ";
                                    uint16_t thirdByte = uncompVec.at(mpdzIndex + 24 + 2);
                                    cout << "thirdByte: " << (int)thirdByte << "; ";
                                    uint16_t screenBaseBlockMaybe = uncompVec.at(mpdzIndex + 24 + 3);
                                    cout << "screenBaseBlockMaybe: " << (int)screenBaseBlockMaybe << endl;

                                    uint32_t unknownValue5 = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 28); // 00000000
                                    cout << "        unk5: " << hex << unknownValue5 << endl;
                                    Q_UNUSED(canvasDimensions);
                                    Q_UNUSED(unknownValue1);
                                    Q_UNUSED(unknownValue2);
                                    Q_UNUSED(unknownValue3);
                                    Q_UNUSED(unknownValue5);
                                    Q_UNUSED(whichBgToWriteTo);
                                    Q_UNUSED(charBaseBlockHardMaybe);
                                    Q_UNUSED(thirdByte);
                                    Q_UNUSED(screenBaseBlockMaybe);
                                    uint32_t infoLength = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 4); // First time: 0x20
                                    if (infoLength > 0x18) {
                                        // Get charfile string
                                        auto charFileNoExt = YUtils::getNullTermTextFromVec(uncompVec, mpdzIndex + 32);
                                        cout << "        IMBZ: " << charFileNoExt << endl;
                                    }
                                    break;
                                }
                                case Constants::PLTB_MAGIC_NUM: {
                                    cout << "PLTB" << endl;
                                    break;
                                }
                                case Constants::COLZ_MAGIC_NUM: {
                                    cout << "COLZ" << endl;
                                    break;
                                }
                                case Constants::MPBZ_MAGIC_NUM: {
                                    cout << "MPBZ" << endl;
                                    break;
                                }
                                case Constants::PLAN_MAGIC_NUM: {
                                    cout << "PLAN" << endl;
                                    break;
                                }
                                case Constants::RAST_MAGIC_NUM: {
                                    cout << "RAST"<< endl;
                                    break;
                                }
                                case Constants::IMBZ_MAGIC_NUM: {
                                    cout << "IMBZ" << endl;
                                    break;
                                }
                                case Constants::SCRL_MAGIC_NUM: {
                                    cout << "SCRL" << endl;
                                    break;
                                }
                                default: {
                                    cout << "Unknown: " << hex << curSubInstruction << endl;
                                    break;
                                }
                            }
                            uint32_t imgbLength = YUtils::getUint32FromVec(uncompVec, mpdzIndex + 4);
                            // TODO: Learn about this data
                            mpdzIndex += imgbLength + 8;
                        }
                    } else if (curInstruction == Constants::GRAD_MAGIC_NUM) {
                        cout << "    GRAD" << endl;
                        mpdzIndex += 4; // Go to length
                        auto gradLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        mpdzIndex += 4; // Now at start of actual data

                        // Do stuff here
                        // For now, skip
                        mpdzIndex += gradLength;
                    } else if (curInstruction == Constants::SETD_MAGIC_NUM) {
                        cout << "    SETD" << endl;
                        mpdzIndex += 4; // Go to length
                        auto setdLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        // Now at start of actual data
                        mpdzIndex += 4;
                        const uint32_t indexEnd = mpdzIndex + setdLength;
                        int numberOfLevelObjects = 0;
                        while (mpdzIndex < indexEnd) {
                            LevelObject lo;
                            lo.objectId = YUtils::getUint16FromVec(uncompVec, mpdzIndex + 0);
                            lo.settingsLength = YUtils::getUint16FromVec(uncompVec, mpdzIndex + 2);
                            uint16_t len = lo.settingsLength;
                            if (len > 10) {
                                // Silence this here
                            }
                            lo.xPosition = YUtils::getUint16FromVec(uncompVec, mpdzIndex + 4);
                            lo.yPosition = YUtils::getUint16FromVec(uncompVec, mpdzIndex + 6);
                            
                            mpdzIndex += 8; // This skips to either the settings, or the next object
                            if (len > 0) {
                                while (len > 0) {
                                    //auto curSetting = YUtils::getUint16FromVec(mpdzVec, indexPointer);
                                    mpdzIndex += 2;
                                    len -= 2;
                                }
                            }
                            numberOfLevelObjects++;
                            //YUtils::printLevelObject(lo);
                        }
                        cout << "      LevelObjects loaded: " << dec << numberOfLevelObjects << endl;
                    } else if (curInstruction == Constants::ALPH_MAGIC_NUM) {
                        cout << "    ALPH" << endl;
                        mpdzIndex += 4; // Go to length
                        auto alphLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        mpdzIndex += 4; // Now at start of actual data

                        // Do stuff here
                        // For now, skip
                        mpdzIndex += alphLength;
                    } else if (curInstruction == Constants::AREA_MAGIC_NUM) {
                        cout << "    AREA" << endl;
                        mpdzIndex += 4; // Go to length
                        auto areaLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        mpdzIndex += 4; // Now at start of actual data

                        // Do stuff here
                        // For now, skip
                        mpdzIndex += areaLength;
                    } else if (curInstruction == Constants::PATH_MAGIC_NUM) {
                        cout << "    PATH" << endl;
                        mpdzIndex += 4; // Go to length
                        auto pathLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        mpdzIndex += 4; // Now at start of actual data

                        // Do stuff here
                        // For now, skip
                        mpdzIndex += pathLength;
                    } else if (curInstruction == Constants::BLKZ_MAGIC_NUM) {
                        cout << "    BLKZ" << endl;
                        mpdzIndex += 4; // Go to length
                        auto blkzLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        mpdzIndex += 4; // Now at start of actual data

                        // Do stuff here
                        // For now, skip
                        mpdzIndex += blkzLength;
                    } else if (curInstruction == Constants::BRAK_MAGIC_NUM) {
                        cout << "    BRAK" << endl;
                        mpdzIndex += 4; // Go to length
                        auto brakLength = YUtils::getUint32FromVec(uncompVec,mpdzIndex);
                        mpdzIndex += 4; // Now at start of actual data

                        // Do stuff here
                        // For now, skip
                        mpdzIndex += brakLength;
                    } else {
                        std::stringstream ssInstNotScen;
                        ssInstNotScen << "Instruction besides SCEN used: " << hex << curInstruction;
                        YUtils::printDebug(ssInstNotScen.str(),DebugType::ERROR);
                        return;
                    }
                }

                // +0x4 is because the magic number is 4 long, and the next is the length
                // +0x8 is because that length is added to the current read position in the file
                //   That recreates it as a non-relative offset number (it started from the
                //   maybeExits at +0x8)
                uint32_t cscnLength = this->getNumberAt<uint32_t>(baseAddrCscn + 0x4) + 0x8;
                
                curCscnReadOffset += cscnLength;
            }
        }
    }
}