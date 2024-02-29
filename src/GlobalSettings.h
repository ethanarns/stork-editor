#ifndef SETTINGSENUM_H
#define SETTINGSENUM_H

#include <cstdint>

struct GlobalSettings {
    uint32_t brushWidth;
    uint32_t brushHeight;
};
extern GlobalSettings globalSettings;

#endif