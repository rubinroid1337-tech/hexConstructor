#pragma once
#include <stdint.h>
#include "gate.h"
#include "switch.h"
#include "led.h"
#include "wire.h"

#define MAX_ITEMS 65535

typedef struct {
    Gate gates[MAX_ITEMS];
    Switch switches[MAX_ITEMS];
    Led leds[MAX_ITEMS];
    Wire wires[MAX_ITEMS];

    uint16_t gatesCount;
    uint16_t switchesCount;
    uint16_t ledsCount;
    uint16_t wiresCount;
} Circuit;

void tick(Circuit *c);

