#pragma once

#include <raylib.h>
#include "../core/kind.hpp"

struct PinHit {
    Vector2 pos{};
    int idx = -1;
    Kind type = GATE;
    bool isOut{};
    bool gatePin{};

    explicit operator bool() const { return idx >= 0; }
};
