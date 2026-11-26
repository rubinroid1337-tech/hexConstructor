#pragma once
#include <stdint.h>
#include <raylib.h>
#include "../core/wire.h"
#include "../core/gate-type.h"

typedef struct {
    Vector2 *gates;
    GateType *gateTypes;
    uint16_t gatesCount;

    Vector2 *switches;
    uint16_t switchesCount;

    Vector2 *leds;
    uint16_t ledsCount;

    Wire *wires;
    uint16_t wiresCount;
} Clipboard;

void clipboard_clear(Clipboard *cb);
[[nodiscard]] bool clipboard_empty(const Clipboard *cb);

