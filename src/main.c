#include <stdio.h>
#include <assert.h>
#include "core/circuit.h"

int main(void) {
    static Circuit c = {0};

    c.switchesCount = 2;
    c.gates[0] = (Gate){ .type = GATE_AND };
    c.gatesCount = 1;
    c.ledsCount = 1;

    c.wires[0] = (Wire){ .fromType = SWITCH, .from = 0, .toType = GATE, .to = 0, .pin = 0 };
    c.wires[1] = (Wire){ .fromType = SWITCH, .from = 1, .toType = GATE, .to = 0, .pin = 1 };
    c.wires[2] = (Wire){ .fromType = GATE,   .from = 0, .toType = LED,  .to = 0, .pin = 0 };
    c.wiresCount = 3;

    tick(&c);
    tick(&c);
    assert(c.leds[0].on == false);

    c.switches[0].on = true;
    tick(&c);
    tick(&c);
    assert(c.leds[0].on == false);

    c.switches[1].on = true;
    tick(&c);
    tick(&c);
    assert(c.leds[0].on == true);

    printf("Тесты пройдены! LED горит: %d\n", c.leds[0].on);
}

