#include "LevelObject.h"
#include "utils.h"

#include <vector>
#include <iostream>

#include <QByteArray>

using namespace std;

ObjectGraphicMetadata LevelObject::getObjectGraphicMetadata(uint16_t objectId) {
    ObjectGraphicMetadata meta;
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
        case 0x3b: { // Red coin
            meta.tilesSectorOffset = 0;
            meta.paletteSectorOffset = 0;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            meta.subTile = 6;
            break;
        }
        case 0x9A: { // Red sign
            meta.tilesSectorOffset = 0x5A;
            meta.paletteSectorOffset = 0xDC;
            meta.tilesCount = 12;
            meta.tileWidth = 4;
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
            meta.paletteSectorOffset = 0xa8; // Is in objeffect.arcz
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