#pragma once
#include <string.h>
#include "../core/kind.h"

typedef struct {
    bool gates[65535];
    bool switches[65535];
    bool leds[65535];
    int count;
} Selection;

void selection_clear(Selection *s);
bool selection_empty(const Selection *s);
bool selection_has(const Selection *s, Kind type, int idx);

