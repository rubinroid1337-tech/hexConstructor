#include "gate.h"

bool isSingleInput(const Gate *g) {
    return g->type == GATE_NOT;
}

void evaluate(Gate *g) {
    switch (g->type) {
        case GATE_AND: g->out = (g->in[0] && g->in[1]); break;
        case GATE_OR:  g->out = (g->in[0] || g->in[1]); break;
        case GATE_XOR: g->out = (g->in[0] != g->in[1]); break;
        case GATE_NOT: g->out = !g->in[0];              break;
    }
}

const char *label(const Gate *g) {
    switch (g->type) {
        case GATE_AND: return "AND";
        case GATE_OR:  return "OR";
        case GATE_XOR: return "XOR";
        case GATE_NOT: return "NOT";
    }
    return "?";
}
