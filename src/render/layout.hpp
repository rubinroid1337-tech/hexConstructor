#pragma once

#include <vector>
#include <raylib.h>

struct Layout {
    std::vector<Vector2> gates;
    std::vector<Vector2> switches;
    std::vector<Vector2> leds;
};
