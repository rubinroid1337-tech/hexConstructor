#include "hotkeys-system.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raymath.h>
#include "utils.h"

static inline void sel_set(Selection *s, Kind type, int idx, bool val) {
    if ((unsigned)idx >= 65535) return;
    bool *arr = (type == GATE) ? s->gates : (type == SWITCH ? s->switches : s->leds);
    if (arr[idx] != val) {
        arr[idx] = val;
        s->count += val ? 1 : -1;
    }
}

static int find_local_idx(const bool *sel, uint16_t total, int target) {
    int local = 0;
    for (int i = 0; i < total; ++i) {
        if (!sel[i]) continue;
        if (i == target) return local;
        local++;
    }
    return -1;
}

static void copy_selection(Editor *ed, const Circuit *c, const Layout *pos) {
    clipboard_clear(&ed->clipboard);
    Clipboard *cb = &ed->clipboard;

    uint16_t gCnt = 0, sCnt = 0, lCnt = 0;
    Vector2 center = {};

    for (int i = 0; i < c->gatesCount; ++i) {
        if (ed->selection.gates[i]) { center = Vector2Add(center, pos->gates[i]); gCnt++; }
    }
    for (int i = 0; i < c->switchesCount; ++i) {
        if (ed->selection.switches[i]) { center = Vector2Add(center, pos->switches[i]); sCnt++; }
    }
    for (int i = 0; i < c->ledsCount; ++i) {
        if (ed->selection.leds[i]) { center = Vector2Add(center, pos->leds[i]); lCnt++; }
    }

    int total = gCnt + sCnt + lCnt;
    if (total == 0) return;
    center = Vector2Scale(center, 1.0f / (float)total);

    if (gCnt > 0) {
        cb->gates = malloc(gCnt * sizeof(Vector2));
        cb->gateTypes = malloc(gCnt * sizeof(GateType));
        for (int i = 0; i < c->gatesCount; ++i) {
            if (!ed->selection.gates[i]) continue;
            cb->gates[cb->gatesCount] = Vector2Subtract(pos->gates[i], center);
            cb->gateTypes[cb->gatesCount] = c->gates[i].type;
            cb->gatesCount++;
        }
    }
    if (sCnt > 0) {
        cb->switches = malloc(sCnt * sizeof(Vector2));
        for (int i = 0; i < c->switchesCount; ++i) {
            if (!ed->selection.switches[i]) continue;
            cb->switches[cb->switchesCount++] = Vector2Subtract(pos->switches[i], center);
        }
    }
    if (lCnt > 0) {
        cb->leds = malloc(lCnt * sizeof(Vector2));
        for (int i = 0; i < c->ledsCount; ++i) {
            if (!ed->selection.leds[i]) continue;
            cb->leds[cb->ledsCount++] = Vector2Subtract(pos->leds[i], center);
        }
    }

    uint16_t wCap = 0;
    for (int i = 0; i < c->wiresCount; ++i) {
        const Wire *w = &c->wires[i];
        const bool *fromSel = (w->fromType == GATE) ? ed->selection.gates : (w->fromType == SWITCH ? ed->selection.switches : ed->selection.leds);
        const bool *toSel   = (w->toType == GATE)   ? ed->selection.gates : (w->toType == SWITCH   ? ed->selection.switches : ed->selection.leds);
        uint16_t fromTot    = (w->fromType == GATE) ? c->gatesCount       : (w->fromType == SWITCH ? c->switchesCount       : c->ledsCount);
        uint16_t toTot      = (w->toType == GATE)   ? c->gatesCount       : (w->toType == SWITCH   ? c->switchesCount       : c->ledsCount);

        int a = find_local_idx(fromSel, fromTot, w->from);
        int b = find_local_idx(toSel, toTot, w->to);
        if (a != -1 && b != -1) {
            if (cb->wiresCount == wCap) {
                wCap = wCap == 0 ? 16 : wCap * 2;
                cb->wires = realloc(cb->wires, wCap * sizeof(Wire));
            }
            cb->wires[cb->wiresCount++] = (Wire){
                .from = a, .to = b,
                .fromType = w->fromType, .toType = w->toType,
                .pin = w->pin, .on = false
            };
        }
    }
}

static void paste_clipboard(Editor *ed, Circuit *c, Layout *pos, Vector2 mouse) {
    const Clipboard *cb = &ed->clipboard;
    if (clipboard_empty(cb)) return;
    if (c->gatesCount + cb->gatesCount >= MAX_ITEMS ||
        c->switchesCount + cb->switchesCount >= MAX_ITEMS ||
        c->ledsCount + cb->ledsCount >= MAX_ITEMS ||
        c->wiresCount + cb->wiresCount >= MAX_ITEMS) return;

    editor_push_history(ed, c, pos);
    selection_clear(&ed->selection);

    int base[3] = { c->gatesCount, c->switchesCount, c->ledsCount };

    for (uint16_t i = 0; i < cb->gatesCount; ++i) {
        int idx = c->gatesCount++;
        sel_set(&ed->selection, GATE, idx, true);
        c->gates[idx] = (Gate){ .type = cb->gateTypes[i] };
        pos->gates[idx] = Vector2Add(mouse, cb->gates[i]);
    }
    for (uint16_t i = 0; i < cb->switchesCount; ++i) {
        int idx = c->switchesCount++;
        sel_set(&ed->selection, SWITCH, idx, true);
        c->switches[idx] = (Switch){};
        pos->switches[idx] = Vector2Add(mouse, cb->switches[i]);
    }
    for (uint16_t i = 0; i < cb->ledsCount; ++i) {
        int idx = c->ledsCount++;
        sel_set(&ed->selection, LED, idx, true);
        c->leds[idx] = (Led){};
        pos->leds[idx] = Vector2Add(mouse, cb->leds[i]);
    }
    for (uint16_t i = 0; i < cb->wiresCount; ++i) {
        Wire w = cb->wires[i];
        c->wires[c->wiresCount++] = (Wire){
            .from = w.from + base[w.fromType],
            .to = w.to + base[w.toType],
            .fromType = w.fromType,
            .toType = w.toType,
            .pin = w.pin,
            .on = false
        };
    }
}

static void spawn_gate(Editor *ed, Circuit *c, Layout *pos, Vector2 mouse, GateType t) {
    if (c->gatesCount >= MAX_ITEMS) return;
    editor_push_history(ed, c, pos);
    int idx = c->gatesCount++;
    c->gates[idx] = (Gate){ .type = t };
    pos->gates[idx] = (Vector2){ mouse.x - 30, mouse.y - 20 };
}

static int hit_rect_list(const Vector2 *pts, uint16_t count, Vector2 mouse, float w, float h) {
    for (int i = (int)count - 1; i >= 0; --i) {
        Rectangle r = { pts[i].x, pts[i].y, w, h };
        if (CheckCollisionPointRec(mouse, r)) return i;
    }
    return -1;
}

bool hotkeys_update(Editor *ed, Circuit *c, Layout *pos) {
    Vector2 screenMouse = GetMousePosition();
    Vector2 mouse = GetScreenToWorld2D(screenMouse, ed->camera);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    bool space = IsKeyDown(KEY_SPACE);
    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

    if (ctrl && IsKeyPressed(KEY_Z)) {
        if (shift) editor_redo(ed, c, pos);
        else editor_undo(ed, c, pos);
        return false;
    }
    if (ctrl && IsKeyPressed(KEY_Y)) { editor_redo(ed, c, pos); return false; }
    if (ctrl && IsKeyPressed(KEY_C) && !selection_empty(&ed->selection)) copy_selection(ed, c, pos);
    if (ctrl && IsKeyPressed(KEY_V) && !clipboard_empty(&ed->clipboard)) paste_clipboard(ed, c, pos, mouse);
    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        if (IsKeyPressed(KEY_ESCAPE)) {
        if (pinhit_valid(ed->wireStart)) { ed->wireStart = pinhit_empty(); return false; }
        if (!selection_empty(&ed->selection)) { selection_clear(&ed->selection); return false; }
        return true;
    }

    if (space && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ed->isPanning = true;
        ed->panLastMouse = screenMouse;
        return false;
    }
    if (ed->isPanning) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = Vector2Subtract(screenMouse, ed->panLastMouse);
            ed->camera.target = Vector2Subtract(ed->camera.target, Vector2Scale(delta, 1.0f / ed->camera.zoom));
            ed->panLastMouse = screenMouse;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            return false;
        }
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        ed->isPanning = false;
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 worldBefore = GetScreenToWorld2D(screenMouse, ed->camera);
        ed->camera.offset = screenMouse;
        ed->camera.target = worldBefore;
        float factor = 1.0f + (0.15f * fabsf(wheel));
        if (wheel < 0.0f) factor = 1.0f / factor;
        ed->camera.zoom = Clamp(ed->camera.zoom * factor, 0.2f, 4.0f);
    }

    if (IsKeyPressed(KEY_Q)) spawn_gate(ed, c, pos, mouse, GATE_AND);
    if (IsKeyPressed(KEY_W)) spawn_gate(ed, c, pos, mouse, GATE_OR);
    if (IsKeyPressed(KEY_E)) spawn_gate(ed, c, pos, mouse, GATE_XOR);
    if (IsKeyPressed(KEY_R)) spawn_gate(ed, c, pos, mouse, GATE_NOT);
    if (IsKeyPressed(KEY_ONE) && c->switchesCount < MAX_ITEMS) {
        editor_push_history(ed, c, pos);
        int i = c->switchesCount++;
        c->switches[i] = (Switch){};
        pos->switches[i] = (Vector2){ mouse.x - 20, mouse.y - 20 };
    }
    if (IsKeyPressed(KEY_TWO) && c->ledsCount < MAX_ITEMS) {
        editor_push_history(ed, c, pos);
        int i = c->ledsCount++;
        c->leds[i] = (Led){};
        pos->leds[i] = (Vector2){ mouse.x - 20, mouse.y - 20 };
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PinHit hit = findPin(mouse, c, pos);
        if (!pinhit_valid(ed->wireStart) && shift && pinhit_valid(hit)) {
            ed->wireStart = hit;
            return false;
        } else if (pinhit_valid(ed->wireStart)) {
            bool diffNode = !(hit.type == ed->wireStart.type && hit.idx == ed->wireStart.idx);
            if (pinhit_valid(hit) && hit.isOut != ed->wireStart.isOut && diffNode && c->wiresCount < MAX_ITEMS) {
                editor_push_history(ed, c, pos);
                PinHit outPin = ed->wireStart.isOut ? ed->wireStart : hit;
                PinHit inPin  = ed->wireStart.isOut ? hit : ed->wireStart;
                c->wires[c->wiresCount++] = (Wire){
                    .from = outPin.idx, .to = inPin.idx,
                    .fromType = outPin.type, .toType = inPin.type,
                    .pin = inPin.gatePin, .on = false
                };
            }
            ed->wireStart = pinhit_empty();
            return false;
        }

        if (!space) {
            ed->clickStartWorld = ed->dragLastWorld = mouse;
            Kind hitType = GATE;
            int hitIdx = hit_rect_list(pos->gates, c->gatesCount, mouse, 60, 40);
            if (hitIdx < 0) { hitType = SWITCH; hitIdx = hit_rect_list(pos->switches, c->switchesCount, mouse, 40, 40); }
            if (hitIdx < 0) { hitType = LED;    hitIdx = hit_rect_list(pos->leds, c->ledsCount, mouse, 40, 40); }

            if (hitIdx >= 0) {
                editor_push_history(ed, c, pos);
                ed->isDragging = true;
                ed->isSingleDrag = !selection_has(&ed->selection, hitType, hitIdx);
                if (ed->isSingleDrag) {
                    selection_clear(&ed->selection);
                    sel_set(&ed->selection, hitType, hitIdx, true);
                }
            } else {
                selection_clear(&ed->selection);
                ed->isSelecting = true;
                ed->isSingleDrag = false;
                ed->selectStart = mouse;
                ed->selectBox = (Rectangle){ mouse.x, mouse.y, 0, 0 };
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (ed->isDragging) {
            Vector2 delta = Vector2Subtract(mouse, ed->dragLastWorld);
            for (int i = 0; i < c->gatesCount; ++i)    if (ed->selection.gates[i])    pos->gates[i]    = Vector2Add(pos->gates[i], delta);
            for (int i = 0; i < c->switchesCount; ++i) if (ed->selection.switches[i]) pos->switches[i] = Vector2Add(pos->switches[i], delta);
            for (int i = 0; i < c->ledsCount; ++i)     if (ed->selection.leds[i])     pos->leds[i]     = Vector2Add(pos->leds[i], delta);
            ed->dragLastWorld = mouse;
        } else if (ed->isSelecting) {
            ed->selectBox = (Rectangle){
                fminf(ed->selectStart.x, mouse.x), fminf(ed->selectStart.y, mouse.y),
                fabsf(mouse.x - ed->selectStart.x), fabsf(mouse.y - ed->selectStart.y)
            };
            selection_clear(&ed->selection);
            for (int i = 0; i < c->gatesCount; ++i)    if (CheckCollisionRecs(ed->selectBox, (Rectangle){pos->gates[i].x, pos->gates[i].y, 60, 40}))    sel_set(&ed->selection, GATE, i, true);
            for (int i = 0; i < c->switchesCount; ++i) if (CheckCollisionRecs(ed->selectBox, (Rectangle){pos->switches[i].x, pos->switches[i].y, 40, 40})) sel_set(&ed->selection, SWITCH, i, true);
            for (int i = 0; i < c->ledsCount; ++i)     if (CheckCollisionRecs(ed->selectBox, (Rectangle){pos->leds[i].x, pos->leds[i].y, 40, 40}))       sel_set(&ed->selection, LED, i, true);
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (ed->isDragging) {
            if (Vector2Distance(ed->clickStartWorld, mouse) < 4.0f) {
                (void)history_pop(&ed->undoStack, nullptr);
                if (ed->selection.count == 1) {
                    for (int i = 0; i < c->switchesCount; ++i) {
                        if (ed->selection.switches[i]) { c->switches[i].on = !c->switches[i].on; break; }
                    }
                }
            }
            if (ed->isSingleDrag) selection_clear(&ed->selection);
        }
        ed->isDragging = ed->isSelecting = ed->isSingleDrag = false;
    }

    if (IsKeyPressed(KEY_X)) {
        if (!selection_empty(&ed->selection)) {
            editor_push_history(ed, c, pos);
            for (int i = (int)c->gatesCount - 1; i >= 0; --i)    if (ed->selection.gates[i])    deleteNode(c, pos, GATE, i);
            for (int i = (int)c->switchesCount - 1; i >= 0; --i) if (ed->selection.switches[i]) deleteNode(c, pos, SWITCH, i);
            for (int i = (int)c->ledsCount - 1; i >= 0; --i)     if (ed->selection.leds[i])     deleteNode(c, pos, LED, i);
            selection_clear(&ed->selection);
            ed->wireStart = pinhit_empty();
            return false;
        }
        for (int i = (int)c->wiresCount - 1; i >= 0; --i) {
            const Wire *w = &c->wires[i];
            Vector2 start = (w->fromType == SWITCH)
                ? (Vector2){ pos->switches[w->from].x + 40, pos->switches[w->from].y + 20 }
                : (Vector2){ pos->gates[w->from].x + 60, pos->gates[w->from].y + 20 };
            Vector2 end = (w->toType == LED)
                ? (Vector2){ pos->leds[w->to].x, pos->leds[w->to].y + 20 }
                : (Vector2){ pos->gates[w->to].x, pos->gates[w->to].y + (isSingleInput(&c->gates[w->to]) ? 20.0f : (w->pin ? 30.0f : 10.0f)) };
            if (isMouseOverWire(mouse, start, end)) {
                editor_push_history(ed, c, pos);
                if (i < (int)c->wiresCount - 1) memmove(&c->wires[i], &c->wires[i + 1], (size_t)((int)c->wiresCount - 1 - i) * sizeof(Wire));
                c->wiresCount--;
                return false;
            }
        }
    }
    return false;
}



