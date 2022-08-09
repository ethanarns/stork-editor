#ifndef LEVELOBJECT_H
#define LEVELOBJECT_H

#include <cstdint>
#include <vector>

#include <QByteArray>

struct ObjectDrawInstruction {
    QByteArray pixelVector;
    int16_t offsetX;
    int16_t offsetY;
    uint16_t animationHoldTime;
};

class LevelObject {
public:
    uint16_t objectId;
    uint16_t settingsLength;
    uint16_t xPosition;
    uint16_t yPosition;
    static std::vector<ObjectDrawInstruction> getInstructionsFromObjsetRecord(std::vector<uint8_t> dataVector, uint32_t objectOffset);
};
#endif