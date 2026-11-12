#pragma once

#include <cstdint>
#include <vector>

#include "ware.hpp"
#include "gate.hpp"

struct GateSystem {
    std::vector<Gate> gates;
    std::vector<Wire> wires;

    void updateAllGates() {
        for (const auto& w : wires) {
            uint8_t signal = gates[w.fromGateIndex].output;
            if (w.inputPin == 0) {
                gates[w.toGateIndex].inputA = signal;
            } else {
                gates[w.toGateIndex].inputB = signal;
            }
        }

        for (auto& gate : gates) {
            gate.update();
        }
    }

    int addGate() {
        gates.emplace_back();
        return static_cast<int>(gates.size()) - 1;
    }

    void addWire(int from, int to, uint8_t pin) {
        wires.push_back({
            static_cast<uint16_t>(from),
            static_cast<uint16_t>(to),
            pin
        });
    }
};
