#pragma once

#include <vector>

struct Selection {
    std::vector<int> gates;
    std::vector<int> switches;
    std::vector<int> leds;

    void clear() { gates.clear(); switches.clear(); leds.clear(); }
    bool empty() const { return gates.empty() && switches.empty() && leds.empty(); }
    bool has(Kind type, int idx) const {
        const auto& v = (type == GATE) ? gates : (type == SWITCH ? switches : leds);
        return std::find(v.begin(), v.end(), idx) != v.end();
    }
};
