#include "LevelObject.h"
#include "utils.h"

#include <vector>
#include <iostream>

#include <QByteArray>

using namespace std;

ObjectGraphicMetadata LevelObject::getObjectGraphicMetadata(LevelObject lo) {
    ObjectGraphicMetadata meta;
    auto objectId = lo.objectId;
    meta.subTile = 0;
    meta.whichPaletteFile = PaletteFileName::OBJSET;
    switch(objectId) {
        case 0x0: { // Basic yellow coin
            meta.tilesSectorOffset = 0;
            meta.paletteSectorOffset = 0;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        case 0x13: { // Question mark cloud?
            meta.tilesCount = 0; // Needs special rendering...
            break;
        }
        case 0x23: { // Green pipe
            constexpr uint32_t VERTICAL_PIPE_TILES = 0x13;
            constexpr uint32_t HORIZONTAL_PIPE_TILES = 0x12;
            YUtils::printVector(lo.settings);
            uint32_t pipeHeight = (uint32_t)lo.settings.at(2);
            meta.tilesSectorOffset = VERTICAL_PIPE_TILES;
            meta.paletteSectorOffset = 0x89;
            meta.tilesCount = 4 * pipeHeight;
            meta.tileWidth = 4;
            meta.subTile = 0;
            // TODO: Special rendering to repeat downwards
            break;
        }
        case 0x28: { // Flower
            meta.tilesSectorOffset = 0x16;
            meta.paletteSectorOffset = 0x9b;
            meta.tilesCount = 16;
            meta.tileWidth = 4;
            break;
        }
        case 0x2b: { // Poundable Pillar
            //YUtils::printVector(lo.settings);
            meta.tilesSectorOffset = 0x17;
            meta.paletteSectorOffset = 0x8d;
            meta.tileWidth = 2;
            meta.tilesCount = 4;
            // TODO: How far can it repeat down?
            break;
        }
        case 0x2c: { // One-way gate
            // TODO: Render bottom half as mirror and handle reverse
            meta.tilesSectorOffset = 0x19;
            meta.paletteSectorOffset = 0x8e;
            meta.tileWidth = 2;
            meta.tilesCount = 2 * 6;
            break;
        }
        case 0x3b: { // Red coin
            meta.tilesSectorOffset = 0;
            meta.paletteSectorOffset = 0;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            meta.subTile = 6;
            break;
        }
        case 0x3c: { // Timer Block
            meta.tilesSectorOffset = 0x1e;
            meta.paletteSectorOffset = 0xe3;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        case 0x4e: { // Dandylion
            meta.tilesSectorOffset = 0x2f;
            meta.paletteSectorOffset = 0x8f;
            meta.tilesCount = 4*6;
            meta.tileWidth = 4;
            break;
        }
        case 0x94: { // Chomp-breakable blocks
            // Fuck. They're an arc file. "objsbblock.arc"
            // TODO: Implement objsbblock.arc
            meta.tilesCount = 0;
            break;
        }
        case 0x9A: { // Red arrow signs
            meta.tilesSectorOffset = 0x5A;
            meta.paletteSectorOffset = 0xDC;
            // Defaults to classic red arrow sign
            meta.tilesCount = 12;
            meta.tileWidth = 4;
            if (lo.settingsLength < 2) {
                std::cerr << "[ERROR] Red Sign needs at least 2 bytes of settings, instead got " << lo.settingsLength << endl;
                break;
            }
            auto firstSettingsByte = (uint32_t)lo.settings.at(0);
            meta.subTile = firstSettingsByte;
            switch(firstSettingsByte) {
                case 0x1: { // Classic right pointing signpost
                    meta.tilesCount = 3 * 4;
                    meta.tileWidth = 4;
                    break;
                }
                case 0x5: { // Down right
                    // TODO: ROTATED, NEEDS SPECIAL RENDER
                    meta.tilesCount = 4;
                    meta.tileWidth = 2;
                    meta.subTile = 3;
                    break;
                }
                case 0x6: { // Upwards pointing arrow
                    meta.tilesCount = 6;
                    meta.tileWidth = 2;
                    break;
                }
                default: {
                    std::cout << "[WARN] Unhandled red sign type: " << hex << firstSettingsByte << endl;
                    break;
                }
            }
            break;
        }
        case 0x9f: { // Info/hint block
            meta.tilesSectorOffset = 0x5d;
            meta.paletteSectorOffset = 0xa9;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        case 0xa1: { // Block that expands when you hit it
            meta.tilesSectorOffset = 0x5e;
            meta.whichPaletteFile = PaletteFileName::OBJEFFECT;
            meta.paletteSectorOffset = 0x2a-2; // Is in objeffect.arcz, strange offset
            // 02096680 is where the function to get the palette address is
            // Actual function is at 020e448c. Doesn't SEEM to have any offset, but might skip the first few?
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        case 0xd2: { // Yellow egg block
            meta.tilesSectorOffset = 0x1;
            meta.whichPaletteFile = PaletteFileName::OBJEFFECT;
            meta.paletteSectorOffset = 0x2a-2;
            meta.subTile = 4; // TODO: Get from object data values
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        case 0xe7: { // M Block that only shows up when carrying baby Mario
            meta.tilesSectorOffset = 0x5f;
            meta.paletteSectorOffset = 0xe3;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        default: {
            meta.tilesSectorOffset = 0;
            meta.paletteSectorOffset = 0;
            meta.tilesCount = 0;
            meta.tileWidth = 0;
            break;
        }
    }
    return meta;
}