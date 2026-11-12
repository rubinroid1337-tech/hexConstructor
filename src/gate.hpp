#pragma once

#include <cstdint>

struct Gate {
    uint8_t inputA : 1;
    uint8_t inputB : 1;
    uint8_t output : 1;

    void update() { output = !(inputA & inputB); }
};
