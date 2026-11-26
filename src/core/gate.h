#pragma once
#include "gate-type.h"

typedef struct {
    GateType type;
    bool in[2];
    bool out;
} Gate;

bool isSingleInput(const Gate *g);
void evaluate(Gate *g);
const char *label(const Gate *g);

