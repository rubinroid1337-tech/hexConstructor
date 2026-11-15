#pragma once

#include <vector>
#include "gate.hpp"
#include "wire.hpp"

struct Circuit {
    std::vector<Gate> gates;
    std::vector<Wire> wires;

    void tick() {
        for (auto& g : gates) {
            g.in[0] = g.in[1] = false;
        }

        for (auto& w : wires) {
            w.state = gates[w.from].out;
            gates[w.to].in[w.pin] |= w.state;
        }

        for (auto& g : gates) {
            g.out = !(g.in[0] && g.in[1]);
        }
    }
};
