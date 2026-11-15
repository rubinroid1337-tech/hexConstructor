#pragma once

#include <cstdint>

struct Wire {
    uint32_t fromGateIndex : 15;
    uint32_t toGateIndex   : 15;
    uint32_t inputPin      : 1;
    uint32_t state         : 1;
};
