#include "LevelObject.h"
#include "utils.h"

#include <vector>
#include <iostream>
#include <sstream>

#include <QByteArray>

ObjectGraphicMetadata LevelObject::getObjectGraphicMetadata(LevelObject lo) {
    ObjectGraphicMetadata meta;
    auto objectId = lo.objectId;
    meta.whichPaletteFile = "objset.arcz";
    meta.whichObjectFile = "objset.arcz";
    switch(objectId) {
        case 0x0: { // Basic yellow coin
            meta.indexOfTiles = 0x0;
            meta.indexOfPalette = 0x7e; // 020267c4
            break;
        }
        case 0x3: { // Green Egg Item
            // Kinda cheaty... Use animation from egg block
            meta.indexOfTiles = 0x1;
            meta.frame = 0x20;
            meta.xPixelOffset = 8;
            meta.yPixelOffset = 32; // Since the egg block animation is super high up
            meta.indexOfPalette = 0xc7; // TODO: find the ACTUAL palette, this looks bad
            break;
        }
        case 0x5: { // Red egg item
            // Kinda cheaty... Use animation from egg block
            meta.indexOfTiles = 0x1;
            meta.frame = 0x20;
            meta.xPixelOffset = 8;
            meta.yPixelOffset = 32; // Since the egg block animation is super high up
            meta.indexOfPalette = 0xe2;
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
        // Cloud drop 0x11: Objset 0x7 ; 0x9a
        case 0x12: { // Star Item
            meta.indexOfTiles = 8;
            meta.indexOfPalette = 0x81;
            break;
        }
        case 0x13: { // Winged Cloud (items)
            meta.indexOfTiles = 9;
            meta.indexOfPalette = 0xa9;
            meta.isLz10 = true;
            meta.frame = 0;
            if (lo.settings.at(1) == 1) {
                // 1 means starts invisible
                meta.indexOfPalette = 0xc5;
            }
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
        // 0x1f is at 0x91
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
            constexpr uint32_t PIPE_PALETTE = 0x89;
            meta.frame = 0;
            meta.indexOfPalette = PIPE_PALETTE;
            auto direction = YUtils::getUint16FromVec(lo.settings,0);
            auto length = YUtils::getUint16FromVec(lo.settings,2);
            if (length == 0) {
                YUtils::printDebug("Pipe length read is 0",DebugType::ERROR);
                break;
            }
            if (direction == 0) {
                // Going down
                meta.indexOfTiles = VERTICAL_PIPE_TILES;
                for (uint i = 0; i < length; i++) {
                    ObjectGraphicMetadata extra;
                    extra.indexOfTiles = VERTICAL_PIPE_TILES;
                    extra.indexOfPalette = PIPE_PALETTE;
                    extra.frame = 1;
                    extra.xPixelOffset = 0;
                    extra.yPixelOffset = (i * 16) + 16;
                    meta.extras.push_back(extra);
                }
                ObjectGraphicMetadata finalExtra;
                finalExtra.indexOfTiles = VERTICAL_PIPE_TILES;
                finalExtra.indexOfPalette = PIPE_PALETTE;
                finalExtra.frame = 2;
                finalExtra.xPixelOffset = 0;
                finalExtra.yPixelOffset = (length * 16) + 16;
                meta.extras.push_back(finalExtra);
            } else if (direction == 1) {
                // Going up
                meta.indexOfTiles = VERTICAL_PIPE_TILES;
                meta.frame = 3;
                meta.yPixelOffset = -16;
                for (uint i = 0; i < length; i++) {
                    ObjectGraphicMetadata extra;
                    extra.indexOfTiles = VERTICAL_PIPE_TILES;
                    extra.indexOfPalette = PIPE_PALETTE;
                    extra.frame = 4;
                    extra.xPixelOffset = 0;
                    extra.yPixelOffset = -16 - (i * 16) - 16; // Second -16 is accounting for base yPixelOffset
                    meta.extras.push_back(extra);
                }
                ObjectGraphicMetadata finalExtra;
                finalExtra.indexOfTiles = VERTICAL_PIPE_TILES;
                finalExtra.indexOfPalette = PIPE_PALETTE;
                finalExtra.frame = 5;
                finalExtra.xPixelOffset = 0;
                finalExtra.yPixelOffset = -16 - (length * 16) - 16;
                meta.extras.push_back(finalExtra);
            } else if (direction == 2) {
                // Going right
                meta.indexOfTiles = HORIZONTAL_PIPE_TILES;
            } else if (direction == 3) {
                // Going left
                meta.indexOfTiles = HORIZONTAL_PIPE_TILES;
                meta.forceFlipH = true;
                meta.xPixelOffset = -16;
            } else {
                std::stringstream ssUnknownDir;
                ssUnknownDir << "Unknown pipe direction: 0x" << std::hex << direction;
                YUtils::printDebug(ssUnknownDir.str(),DebugType::ERROR);
                break;
            }
            break;
        }
        case 0x24: { // Bill Blaster
            meta.indexOfTiles = 0x14;
            meta.indexOfPalette = 0xbf;
            meta.xPixelOffset = 8;
            ObjectGraphicMetadata base;
            base.indexOfTiles = 0x14;
            base.indexOfPalette = 0xbf;
            base.frame = 0x18;
            base.xPixelOffset = 8;
            base.yPixelOffset = 4;
            meta.extras.push_back(base);
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
            ObjectGraphicMetadata extra;
            extra.indexOfTiles = 0x19;
            extra.indexOfPalette = 0x8e;
            uint16_t direction = YUtils::getUint16FromVec(lo.settings,0);
            if (direction == 0) {
                // Right only
                extra.forceFlipV = true;
                extra.yPixelOffset = 16*2;
                meta.extras.push_back(extra);
            } else if (direction == 1) {
                // Left only
                extra.xPixelOffset = -8;
                meta.xPixelOffset = -8;
                meta.forceFlipH = true;
                extra.forceFlipH = true;
                extra.forceFlipV = true;
                extra.yPixelOffset = 16*2;
                meta.extras.push_back(extra);
            } else if (direction == 2) {
                // Down only
                meta.frame = 0x17;
                extra.frame = 0x17;
                extra.xPixelOffset = 16*2;
                extra.forceFlipH = true;
                meta.extras.push_back(extra);
            } else {
                YUtils::printDebug("Unusual gate direction",DebugType::ERROR);
                YUtils::popupAlert("Unusual gate direction");
                break;
            }
            break;
        }
        case 0x2d: { // Winged Cloud (Activator)
            meta.indexOfTiles = 9;
            meta.indexOfPalette = 0xa9;
            meta.isLz10 = true;
            meta.frame = 2; // Just to differ from 0x13
            break;
        }
        case 0x35: { // Goonie Bird
            auto type = YUtils::getUint16FromVec(lo.settings,0);
            auto secondSetting = YUtils::getUint16FromVec(lo.settings,2);
            if (type < 2) { // 0204d1fc
                meta.indexOfTiles = 0x1d; // Living
                meta.indexOfPalette = 0xc9;
            } else {
                meta.indexOfTiles = 0x69; // Undead
                meta.indexOfPalette = 0xf7;
            }
            if (type == 1) {
                // Flightless
                meta.frame = 0x19;
            }
            // TODO: Carrying
            Q_UNUSED(secondSetting);
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
        case 0x44: { // Pointey, test on 2-1 #3
            meta.indexOfTiles = 0x23;
            meta.indexOfPalette = 0xa8;
            if (lo.settings.at(1) == 0x1) {
                // 1 means upside down
                meta.forceFlipV = true;
                meta.yPixelOffset = 16;
            }
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
            ObjectGraphicMetadata extra;
            extra.indexOfTiles = 0x21;
            extra.indexOfPalette = 0x90;
            extra.frame = 1;
            extra.xPixelOffset = 8;
            extra.yPixelOffset = 32;
            meta.extras.push_back(extra);
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
        case 0x1f:
        case 0x91: {
            uint32_t whichPalette = 0x82;
            // 82 is green
            switch (lo.settings.at(0)) {
                case 0x0:
                case 0x1: {
                    whichPalette = 0x84;
                    break;
                }
                case 0x2:
                case 0x3: {
                    whichPalette = 0x82;
                    break;
                }
                case 0x4:
                case 0x5: {
                    whichPalette = 0x85;
                    break;
                }
                case 0x6:
                case 0x7: {
                    whichPalette = 0x83;
                    break;
                }
                default: {
                    whichPalette = 0x81; // Show broken
                    break;
                }
            }
            meta.indexOfTiles = 0x10; // platform used is 0xd
            meta.indexOfPalette = whichPalette;
            meta.xPixelOffset = 4; // May round this up or down
            meta.yPixelOffset = 4; // But it's *actually* 4

            ObjectGraphicMetadata bottom;
            bottom.indexOfTiles = 0xd;
            bottom.indexOfPalette = whichPalette;
            bottom.yPixelOffset = (int32_t)lo.settings.at(1);
            meta.extras.push_back(bottom);

            ObjectGraphicMetadata top;
            top.indexOfTiles = 0xd;
            top.indexOfPalette = whichPalette;
            top.yPixelOffset = ((int32_t)lo.settings.at(1)) * -1;
            meta.extras.push_back(top);

            ObjectGraphicMetadata left;
            left.indexOfTiles = 0xd;
            left.indexOfPalette = whichPalette;
            left.xPixelOffset = ((int32_t)lo.settings.at(1)) * -1;
            meta.extras.push_back(left);

            ObjectGraphicMetadata right;
            right.indexOfTiles = 0xd;
            right.indexOfPalette = whichPalette;
            right.xPixelOffset = (int32_t)lo.settings.at(1);
            meta.extras.push_back(right);
            break;
        }
        // case 0x92: { // Stairs
        //     // TODO: special print me
        //     meta.indexOfTiles = 0x54;
        //     meta.indexOfPalette = 0xe2;
        //     break;
        // }
        case 0x93: { // Stairs Outline
            meta.indexOfTiles = 0x54;
            meta.indexOfPalette = 0xe2;
            meta.frame = 1;
            // Left or right
            if (lo.settings.at(0) == 0) {
                meta.xPixelOffset = 8;
                for (int i = 0; i < 3; i++) {
                    ObjectGraphicMetadata extra;
                    extra.indexOfTiles = 0x54;
                    extra.indexOfPalette = 0xe2;
                    extra.xPixelOffset = i*-8;
                    extra.yPixelOffset = -8 - (i*8); // Negative is upwards
                    extra.frame = 1;
                    meta.extras.push_back(extra);
                }
            } else {
                meta.xPixelOffset = -8;
                for (int i = 0; i < 3; i++) {
                    ObjectGraphicMetadata extra;
                    extra.indexOfTiles = 0x54;
                    extra.indexOfPalette = 0xe2;
                    extra.xPixelOffset = i*8;
                    extra.yPixelOffset = -8 - (i*8); // Negative is upwards
                    extra.frame = 1;
                    meta.extras.push_back(extra);
                }
            }
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
                case 0x7: {
                    meta.frame = 7;
                    meta.yPixelOffset = -8;
                    break;
                }
                case 0x8: {
                    meta.frame = 8;
                    break;
                }
                case 0x9: {
                    meta.frame = 9;
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
        case 0xd1: { // Ice Block
            meta.whichObjectFile = "objiceblock.arcz";
            meta.indexOfTiles = 0x0;
            meta.whichPaletteFile = "objiceblock.arcz";
            meta.indexOfPalette = 0x2;
            // More settings change it
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
        case 0xdf: { // Torch, may only show up in darkness
            meta.whichObjectFile = "objspot.arcz";
            meta.whichPaletteFile = "objspot.arcz";
            meta.indexOfTiles = 1; // 0 appears to be the glow itself
            meta.indexOfPalette = 2;
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
        // 0xf1 fleeper: 0x70 - 0xfe
        case 0x109: { // Toober guy
            meta.indexOfTiles = 0x7b;
            meta.indexOfPalette = 0xa5;
            // TODO: Somehow render 0x7a/0x90 (the shy guy) as well
            break;
        }
        case 0x112: { // Locked minigame hut
            meta.whichObjectFile = "objhouse.arcz";
            meta.whichPaletteFile = "objhouse.arcz";
            meta.indexOfTiles = 0x0;
            meta.indexOfPalette = 0x1;
            meta.xPixelOffset = 8;
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