#ifndef LEVELOBJECT_H
#define LEVELOBJECT_H

#include <cstdint>

struct LevelObject {
    uint16_t objectId;
    uint16_t settingsLength;
    uint16_t xPosition;
    uint16_t yPosition;
};
#endif