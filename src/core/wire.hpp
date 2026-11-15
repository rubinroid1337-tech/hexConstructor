#pragma once

enum NodeType : unsigned char { GATE = 0, SWITCH = 1, LED = 2 };

struct Wire {
    int from{}, to{};
    NodeType fromType = GATE;
    NodeType toType = GATE;
    bool pin{};
    bool on{};
};
