#pragma once

#include <raylib.h>
#include "../core/kind.hpp"

struct PinHit {
    bool valid{};
    bool isOutput{};
    Kind type = GATE;
    int index = -1;
    bool gatePin{};
    Vector2 pos{};
};
