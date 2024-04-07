#include "LevelObject.h"
#include "utils.h"
#include "InstructionRenderer.h"

#include <vector>
#include <iostream>
#include <sstream>

#include <QByteArray>

ObjectGraphicMetadata LevelObject::getObjectGraphicMetadata(LevelObject lo) {
    ObjectGraphicMetadata meta;
    auto objectId = lo.objectId;
    meta.frame = 0;
    meta.whichPaletteFile = "objset.arcz";
    meta.whichObjectFile = "objset.arcz";
    meta.xPixelOffset = 0;
    meta.yPixelOffset = 0;
    meta.indexOfPalette = 0;
    meta.indexOfTiles = 0;
    meta.isLz10 = false;
    meta.is256 = false;
    meta.forceFlipH = false;
    meta.forceFlipV = false;
    switch(objectId) {
        case 0x0: { // Basic yellow coin
            meta.indexOfTiles = 0x0;
            meta.indexOfPalette = 0x7e; // 020267c4
            break;
        }
        case 0x7: { // Egg block (hit for eggs)
            meta.indexOfTiles = 0x1;
            meta.indexOfPalette = 0x89; // Fallback to objset if objeffect doesn't work
            // Which it seems not to
            break;
        }
        case 0xf: { // Pirhana Plant 2
            meta.indexOfTiles = 0x24;
            meta.indexOfPalette = 0x97;
            meta.frame = 1;
            break;
        }
        case 0x10: { // Pirhana Plant 2 (upside down)
            meta.indexOfTiles = 0x24;
            meta.indexOfPalette = 0x97;
            meta.frame = 1;
            meta.yPixelOffset = 32; // push up 2 full tiles
            meta.xPixelOffset = 8;
            meta.forceFlipV = true;
            break;
        }
        case 0x13: { // Winged Cloud (items)
            meta.indexOfTiles = 9;
            meta.indexOfPalette = 0xa9;
            meta.isLz10 = true;
            meta.frame = 0;
            break;
        }
        case 0x14: { // Stork Stop
            meta.indexOfTiles = 0xa;
            meta.indexOfPalette = 0xaa;
            break;
        }
        case 0x1d: { // Spring ball small
            meta.indexOfTiles = 0xf;
            meta.indexOfPalette = 0x86;
            break;
        }
        case 0x1e: { // Spring ball large
            meta.indexOfTiles = 0xe;
            meta.indexOfPalette = 0x86;
            meta.yPixelOffset = -8;
            break;
        }
        case 0x20: { // Falling donuts
            meta.indexOfTiles = 0x11;
            meta.indexOfPalette = 0x87;
            break;
        }
        case 0x21: { // Pirhana Plant 1
            meta.indexOfTiles = 0x24;
            meta.indexOfPalette = 0x97;
            meta.frame = 1;
            meta.xPixelOffset = 8;
            break;
        }
        case 0x22: { // Pirhana Plant 1 Upside down
            meta.indexOfTiles = 0x24;
            meta.indexOfPalette = 0x97;
            meta.frame = 1;
            meta.yPixelOffset = 32; // push up 2 full tiles
            meta.xPixelOffset = 8;
            meta.forceFlipV = true;
            break;
        }
        case 0x23: { // Green pipe
            constexpr uint32_t VERTICAL_PIPE_TILES = 0x13;
            constexpr uint32_t HORIZONTAL_PIPE_TILES = 0x12;
            Q_UNUSED(HORIZONTAL_PIPE_TILES);
            //uint32_t pipeHeight = (uint32_t)lo.settings.at(2)*2; // Doesn't work because data repeats
            meta.indexOfTiles = VERTICAL_PIPE_TILES;
            meta.indexOfPalette = 0x89;
            meta.frame = 0;
            // TODO: Special rendering to repeat downwards
            meta.specialRender = std::vector<RenderInstruction>();
            break;
        }
        case 0x25: { // Red Switch 1
            meta.indexOfTiles = 0x15;
            meta.indexOfPalette = 0x8b;
            break;
        }
        case 0x28: { // Flower
            meta.indexOfTiles = 0x16;
            meta.indexOfPalette = 0x9b;
            break;
        }
        case 0x29: { // Tap Tap (Silver)
            meta.indexOfTiles = 0x18;
            meta.indexOfPalette = 0x98;
            break;
        }
        case 0x2a: { // Tap Tap (Orange)
            meta.indexOfTiles = 0x18;
            meta.indexOfPalette = 0x99;
            break;
        }
        case 0xfb:   // Poundable Pillar trigger (links to 6a often)
        case 0x2b: { // Poundable Pillar
            meta.indexOfTiles = 0x17;
            meta.indexOfPalette = 0x8d;
            // It just goes down until it hits collision
            break;
        }
        case 0x2c: { // One-way gate
            // TODO: Render bottom half as mirror and handle reverse
            meta.indexOfTiles = 0x19;
            meta.indexOfPalette = 0x8e;
            break;
        }
        case 0x2d: { // Winged Cloud (Activator)
            meta.indexOfTiles = 9;
            meta.indexOfPalette = 0xa9;
            meta.isLz10 = true;
            meta.frame = 2; // Just to differ from 0x13
            break;
        }
        case 0x36: { // Yellow Shy Guy
            meta.indexOfTiles = 0x21; // 0202de24
            meta.frame = 0x1;
            meta.indexOfPalette = 0x91; // 0202de18 see above
            // todo: fix x offset?
            break;
        }
        case 0x37: { // Green Shy Guy
            meta.indexOfTiles = 0x21; // 0202de24
            meta.frame = 0x1;
            meta.indexOfPalette = 0x92; // 0202de18 see above
            // todo: fix x offset?
            break;
        }
        case 0x38: { // Blue Shy Guy
            meta.indexOfTiles = 0x21; // 0202de24
            meta.frame = 0x1;
            meta.indexOfPalette = 0x93; // 0202de18 see above
            // todo: fix x offset?
            break;
        }
        case 0x39: { // Red Shy Guy
            meta.indexOfTiles = 0x21; // 0202de24
            meta.frame = 0x1;
            meta.indexOfPalette = 0x90; // 0202de18 see above
            // todo: fix x offset?
            break;
        }
        case 0x3b: { // Red coin
            meta.indexOfTiles = 0;
            meta.indexOfPalette = 0x7e;
            meta.frame = 6;
            break;
        }
        case 0x3c: { // Timer Block
            meta.indexOfTiles = 0x1e;
            meta.indexOfPalette = 0xe3;
            break;
        }
        case 0x40: { // Nipper plant
            meta.indexOfTiles = 0x20;
            meta.indexOfPalette = 0x8f;
            meta.frame = 1;
            break;
        }
        case 0x43: { // Woozy Guy
            meta.indexOfTiles = 0x21;
            meta.indexOfPalette = 0x90;
            meta.frame = 4;
            break;
        }
        case 0x44: { // Pointey
            meta.indexOfTiles = 0x23;
            meta.indexOfPalette = 0xa8;
            break;
        }
        case 0x48: { // Ukiki
            meta.indexOfTiles = 0x26;
            meta.indexOfPalette = 0x9c;
            break;
        }
        case 0x4b: { // Windbag
            meta.indexOfTiles = 0x33;
            meta.indexOfPalette = 0xa1;
            break;
        }
        case 0x4c: { // Glide Guy (umbrella heads)
            meta.indexOfTiles = 0x2b;
            meta.indexOfPalette = 0xa5;
            meta.frame = 1;
            break;
        }
        case 0x4e: { // Dandylion
            meta.indexOfTiles = 0x2f;
            meta.indexOfPalette = 0x8f;
            break;
        }
        case 0x50: { // Poundable Box (stars?)
            meta.indexOfTiles = 0x35;
            meta.indexOfPalette = 0xAD;
            break;
        }
        case 0x54: { // Crayzee Dayzee
            meta.indexOfTiles = 0x37;
            meta.frame = 0;
            meta.indexOfPalette = 0xb0;
            break;
        }
        case 0x7f: { // Fly guy
            meta.indexOfTiles = 0x4b;
            meta.indexOfPalette = 0x90;
            meta.frame = 3;
            break;
        }
        // case 0x81: { // Middle Ring
        //     meta.whichObjectFile = "objsavepointobj.arcz";
        //     break;
        // }
        case 0x8e: { // Large breakable rock squares
            meta.indexOfTiles = 0x48;
            // See 020db27c for other potential palettes
            meta.indexOfPalette = 0xCD;
            break;
        }
        case 0x8f: { // Gooey Goon
            meta.whichObjectFile = "objbutu.arcz";
            meta.whichPaletteFile = "objbutu.arcz";
            meta.indexOfTiles = 0;
            meta.indexOfPalette = 1;
            meta.yPixelOffset = 16;
            break;
        }
        case 0x92: { // Stairs
            // TODO: special print me
            meta.indexOfTiles = 0x54;
            meta.indexOfPalette = 0xe2;
            break;
        }
        case 0x94: { // Chomp-breakable blocks
            // From an arc file: "objsbblock.arc"
            meta.frame = 0;
            meta.indexOfTiles = 0;
            meta.whichObjectFile = "objsbblock.arc";
            meta.whichPaletteFile = "objsbblock.arc";
            meta.indexOfPalette = 2;
            meta.xPixelOffset = 8;
            break;
        }
        case 0x97: { // Ghost Guy
            meta.indexOfTiles = 0x56;
            meta.indexOfPalette = 0xda;
            break;
        }
        case 0x9A: { // Red arrow signs
            meta.indexOfTiles = 0x5A;
            meta.indexOfPalette = 0xDC;
            if (lo.settingsLength < 2) {
                std::stringstream ssRedSignSettingsLen;
                ssRedSignSettingsLen << "Red Sign needs at least 2 bytes of settings, instead got " << lo.settingsLength;
                YUtils::printDebug(ssRedSignSettingsLen.str(),DebugType::ERROR);
                break;
            }
            auto firstSettingsByte = (uint32_t)lo.settings.at(0);
            switch(firstSettingsByte) {
                case 0x0: { // Left pointing, flipped
                    meta.frame = 0;
                    break;
                }
                case 0x1: { // Classic right pointing signpost
                    meta.frame = 1;
                    break;
                }
                case 0x2: { // Up decal
                    meta.frame = 2;
                    meta.yPixelOffset = 8;
                    break;
                }
                case 0x3: { // Up right decal
                    meta.frame = 3;
                    break;
                }
                case 0x4: { // Right decal
                    meta.frame = 4;
                    meta.xPixelOffset = 8;
                    break;
                }
                case 0x5: { // Down right decal
                    meta.frame = 5;
                    meta.yPixelOffset = -8;
                    meta.xPixelOffset = 8;
                    break;
                }
                case 0x6: {
                    meta.frame = 6;
                    break;
                }
                default: {
                    std::stringstream ssUnhandledRedSign;
                    ssUnhandledRedSign << "Unhandled red arrow sign type: " << std::hex << firstSettingsByte;
                    YUtils::printDebug(ssUnhandledRedSign.str(),DebugType::WARNING);
                    meta.indexOfTiles = 0;
                    meta.indexOfPalette = 0;
                    break;
                }
            }
            break;
        }
        case 0x9f: { // Info/hint block
            meta.indexOfTiles = 0x5d;
            meta.indexOfPalette = 0xa9;
            break;
        }
        case 0xa1: { // Block that expands when you hit it
            meta.indexOfTiles = 0x5e;
            meta.whichPaletteFile = "objeffect.arcz";
            meta.indexOfPalette = 0x2a-2; // Is in objeffect.arcz, strange offset
            // 02096680 is where the function to get the palette address is
            // Actual function is at 020e448c. Doesn't SEEM to have any offset, but might skip the first few?
            break;
        }
        case 0xa2: { // Slugger TODO: Test. Where is it located??
            meta.indexOfTiles = 0x5c;
            meta.indexOfPalette = 0xe1;
            break;
        }
        case 0xc4: { // Item-Carrying Balloon
            meta.indexOfTiles = 0x64;
            meta.indexOfPalette = 0xEb;
            break;
        }
        case 0xca: { // Danger-Carrying Balloon
            meta.indexOfTiles = 0x64;
            meta.indexOfPalette = 0xEA;
            break;
        }
        case 0xd2: { // Yellow egg block
            meta.indexOfTiles = 0x1;
            meta.whichPaletteFile = "objeffect.arcz";
            meta.indexOfPalette = 0x2a-2;
            meta.frame = 4; // TODO: Get from object data values
            break;
        }
        case 0xd5: { // Raft
            meta.whichObjectFile = "objraft.arcz";
            meta.whichPaletteFile = "objraft.arcz";
            meta.indexOfTiles = 0x1;
            meta.indexOfPalette = 0x3;
            break;
        }
        case 0xdb: { // Outline coin
            meta.indexOfTiles = 0x0;
            meta.indexOfPalette = 0x7e;
            meta.frame = 12; // Flip to transparency frame
            break;
        }
        case 0xdc: { // ? Bucket, drops coins
            meta.indexOfTiles = 0x6a;
            meta.indexOfPalette = 0xf8;
            break;
        }
        case 0xa5: // Somehow these are duplicates, but this has fewer settings
        case 0xe7: { // M Block that only shows up when carrying baby Mario
            meta.indexOfTiles = 0x5f;
            meta.indexOfPalette = 0xe3;
            break;
        }
        case 0xe9: { // Lantern Ghost, 020ba7f4
            meta.whichObjectFile = "objkantera.arcz";
            meta.whichPaletteFile = "objkantera.arcz";
            meta.indexOfTiles = 0;
            meta.frame = 0;
            meta.indexOfPalette = 1;
            break;
        }
        case 0x109: { // Toober guy
            meta.indexOfTiles = 0x7b;
            meta.indexOfPalette = 0xa5;
            // TODO: Somehow render 0x7a/0x90 (the shy guy) as well
            break;
        }
        case 0x112: { // Locked minigame hut
            // Pulled from "objhouse.arcz"
            break;
        }
        case 0x113: {
            meta.whichObjectFile = "objcharacoin.arc";
            meta.whichPaletteFile = "objcharacoin.arc";
            meta.frame = 0;
            meta.indexOfPalette = 0x6;
            switch(lo.settings.at(0)) {
                case 0x0: {
                    meta.indexOfTiles = 0;
                    break;
                }
                case 0x1: {
                    meta.indexOfTiles = 1;
                    break;
                }
                case 0x2: {
                    meta.indexOfTiles = 2;
                    break;
                }
                default: {
                    meta.indexOfTiles = 4; // Bowser for error
                    break;
                }
            }
            break;
        }
        case 0x116: { // Red Switch 2
            meta.indexOfTiles = 0x15;
            meta.indexOfPalette = 0x8b;
            break;
        }
        case 0x121: { // Pirate Guy
            meta.whichObjectFile = "objpirateheiho.arcz";
            meta.indexOfTiles = 0;
            meta.indexOfPalette = 0x90;
            break;
        }
        default: {
            meta.indexOfTiles = 0;
            meta.indexOfPalette = 0;
            break;
        }
    }
    return meta;
}