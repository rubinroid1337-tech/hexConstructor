#include "selection.h"

void selection_clear(Selection *s) {
    if (s->count == 0) return;
    memset(s, 0, sizeof(Selection));
}

bool selection_empty(const Selection *s) {
    return s->count == 0;
}

bool selection_has(const Selection *s, Kind type, int idx) {
    if ((unsigned)idx >= 65535) return false;
    if (type == GATE)   return s->gates[idx];
    if (type == SWITCH) return s->switches[idx];
    if (type == LED)    return s->leds[idx];
    return false;
}
