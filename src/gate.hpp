#pragma once

struct Gate {
    bool inputA{};
    bool inputB{};
    bool output{};

    void update() { output = !(inputA && inputB); }
};
