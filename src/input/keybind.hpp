#pragma once

#include "raylib.h"

struct KeyBind {
    KeyboardKey primary;
    KeyboardKey secondary = KEY_NULL;

    bool IsDown() const {
        return IsKeyDown(primary) || (secondary != KEY_NULL && IsKeyDown(secondary));
    }

    bool IsPressed() const {
        return IsKeyPressed(primary) || (secondary != KEY_NULL && IsKeyPressed(secondary));
    }
};
