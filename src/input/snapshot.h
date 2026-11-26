#pragma once
#include <stddef.h>
#include "../core/circuit.h"
#include "../render/layout.h"

constexpr size_t MAX_HISTORY = 100;

typedef struct {
    Circuit circuit;
    Layout layout;
} Snapshot;

typedef struct {
    Snapshot *items;
    size_t count;
    size_t capacity;
} HistoryStack;

void history_push(HistoryStack *h, const Circuit *c, const Layout *pos);
[[nodiscard]] bool history_pop(HistoryStack *h, Snapshot *out);
void history_clear(HistoryStack *h);
void history_free(HistoryStack *h);

