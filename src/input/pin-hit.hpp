#pragma once

struct PinHit {
    int id{};
    bool out{};
    bool pin{};
    Vector2 pos{};
    NodeType type = GATE;
};
