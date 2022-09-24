#ifndef INSTRUCTIONRENDERER_H
#define INSTRUCTIONRENDERER_H

#include <cstdint>
#include "Chartile.h"

struct RenderInstruction {
    uint32_t xOffset;
    uint32_t yOffset;
    Chartile tile;
};

#endif