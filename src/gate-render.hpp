#pragma once

#include <raylib.h>
#include <raymath.h>
#include "gate.hpp"

struct GateRender {
    float posX{};
    float posY{};

    void gateRender() {
        DrawCircle(posX, posY, 50.0f, SKYBLUE);
    }
};
