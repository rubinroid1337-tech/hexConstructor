#pragma once
#include <vector>
#include "gate.hpp"
#include "switch.hpp"
#include "led.hpp"
#include "wire.hpp"

struct Circuit {
    std::vector<Gate> gates;
    std::vector<Switch> switches;
    std::vector<Led> leds;
    std::vector<Wire> wires;

    void tick() {
        for (auto& g : gates) g.in[0] = g.in[1] = false;
        for (auto& l : leds)  l.on = false;

        for (auto& w : wires) {
            w.on = (w.fromType == SWITCH) ? switches[w.from].on : gates[w.from].out;

            if (w.toType == LED) {
                leds[w.to].on |= w.on;
            } else {
                gates[w.to].in[w.pin] |= w.on;
            }
        }

        for (auto& g : gates) {
            g.out = !(g.in[0] && g.in[1]);
        }
    }
};
