#pragma once
#include <raylib.h>

typedef struct {
    Vector2 gates[65535];
    Vector2 switches[65535];
    Vector2 leds[65535];
} Layout;
