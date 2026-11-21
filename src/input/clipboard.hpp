#pragma once

#include <vector>
#include <raylib.h>

struct Clipboard {
    std::vector<Vector2> gates;
    std::vector<Vector2> switches;
    std::vector<Vector2> leds;
    std::vector<Wire> wires;
    std::vector<GateType> gateTypes;

    bool empty() const { return gates.empty() && switches.empty() && leds.empty(); }
};
