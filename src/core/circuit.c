#include "circuit.h"
#include <string.h>

static inline bool readOutput(const Circuit *c, Kind type, int idx) {
    if (idx < 0) return false;
    if (type == SWITCH && idx < c->switchesCount) return c->switches[idx].on;
    if (type == GATE   && idx < c->gatesCount)    return c->gates[idx].out;
    return false;
}

static inline void writeInput(Circuit *c, Kind type, int idx, int pin, bool value) {
    if (idx < 0 || !value) return;
    if (type == LED  && idx < c->ledsCount) c->leds[idx].on = true;
    if (type == GATE && idx < c->gatesCount && (unsigned)pin < 2) {
        c->gates[idx].in[pin] = true;
    }
}

void tick(Circuit *c) {
    for (int i = 0; i < c->gatesCount; i++) {
        evaluate(&c->gates[i]);
        c->gates[i].in[0] = c->gates[i].in[1] = false;
    }
    
    memset(c->leds, 0, c->ledsCount * sizeof(Led));

    for (int i = 0; i < c->wiresCount; i++) {
        Wire *w = &c->wires[i];
        w->on = readOutput(c, w->fromType, w->from);
        writeInput(c, w->toType, w->to, w->pin, w->on);
    }
}

