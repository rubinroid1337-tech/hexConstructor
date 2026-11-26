#pragma once
#include <raylib.h>
#include "../core/kind.h"

typedef struct {
    Vector2 pos;
    int idx;
    Kind type;
    bool isOut;
    bool gatePin;
} PinHit;

static inline PinHit pinhit_empty() {
    return (PinHit){ .idx = -1, .type = GATE };
}

static inline bool pinhit_valid(PinHit p) {
    return p.idx >= 0;
}

