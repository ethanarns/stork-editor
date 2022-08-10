#include "LevelObject.h"
#include "utils.h"

#include <vector>
#include <iostream>

#include <QByteArray>

using namespace std;

ObjectGraphicMetadata LevelObject::getObjectGraphicMetadata(uint16_t objectId) {
    ObjectGraphicMetadata meta;
    switch(objectId) {
        case 0x0: {
            meta.tilesSectorOffset = 0;
            meta.paletteSectorOffset = 0;
            meta.tilesCount = 4;
            meta.tileWidth = 2;
            break;
        }
        case 0x9A: {
            meta.tilesSectorOffset = 0x5A;
            meta.paletteSectorOffset = 0xDC;
            meta.tilesCount = 12;
            meta.tileWidth = 4;
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