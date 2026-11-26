#include "clipboard.h"
#include <stdlib.h>
#include <string.h>

void clipboard_clear(Clipboard *cb) {
    free(cb->gates);
    free(cb->gateTypes);
    free(cb->switches);
    free(cb->leds);
    free(cb->wires);
    *cb = (Clipboard){};
}

bool clipboard_empty(const Clipboard *cb) {
    return cb->gatesCount == 0 && cb->switchesCount == 0 && cb->ledsCount == 0;
}

