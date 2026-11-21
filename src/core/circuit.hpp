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

    bool readOutput(Kind type, int idx) const {
        if (idx < 0) return false;
        if (type == SWITCH && idx < (int)switches.size()) return switches[idx].on;
        if (type == GATE   && idx < (int)gates.size())    return gates[idx].out;
        return false;
    }

    void writeInput(Kind type, int idx, int pin, bool value) {
        if (idx < 0 || !value) return;
        if (type == LED  && idx < (int)leds.size()) leds[idx].on = true;
        if (type == GATE && idx < (int)gates.size() && (pin == 0 || pin == 1)) {
            gates[idx].in[pin] = true;
        }
    }

    void tick() {
        for (auto& g : gates) g.in[0] = g.in[1] = false;
        for (auto& l : leds)  l.on = false;

        for (auto& w : wires) {
            w.on = readOutput(w.fromType, w.from);
            writeInput(w.toType, w.to, w.pin, w.on);
        }

        for (auto& g : gates) {
            g.evaluate();
        }
    }
};

