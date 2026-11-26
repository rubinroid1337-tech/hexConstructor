#include "snapshot.h"
#include <stdlib.h>
#include <string.h>

void history_push(HistoryStack *h, const Circuit *c, const Layout *pos) {
    if (h->count >= MAX_HISTORY) {
        memmove(h->items, h->items + 1, (h->count - 1) * sizeof(Snapshot));
        h->count--;
    } else if (h->count == h->capacity) {
        size_t newCap = h->capacity == 0 ? 8 : h->capacity * 2;
        if (newCap > MAX_HISTORY) newCap = MAX_HISTORY;
        Snapshot *tmp = realloc(h->items, newCap * sizeof(Snapshot));
        if (tmp == nullptr) return;
        h->items = tmp;
        h->capacity = newCap;
    }

    h->items[h->count] = (Snapshot){ .circuit = *c, .layout = *pos };
    h->count++;
}

bool history_pop(HistoryStack *h, Snapshot *out) {
    if (h->count == 0) return false;
    h->count--;
    if (out != nullptr) *out = h->items[h->count];
    return true;
}

void history_clear(HistoryStack *h) {
    h->count = 0;
}

void history_free(HistoryStack *h) {
    free(h->items);
    *h = (HistoryStack){};
}

