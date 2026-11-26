#pragma once

#include "kind.h"

typedef struct {
    int from;
    int to;
    Kind fromType;
    Kind toType;
    bool pin;
    bool on;
} Wire;
