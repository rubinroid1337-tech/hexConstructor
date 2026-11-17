#pragma once

#include "kind.hpp"

struct Wire {
    int from{}, to{};
    Kind fromType = GATE;
    Kind toType = GATE;
    bool pin{};
    bool on{};
};
