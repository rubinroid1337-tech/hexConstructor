#include "utils.h"
#include <math.h>
#include <string.h>
#include <raymath.h>

#define ARRAY_REMOVE_AT(arr, count, idx) do { \
    if ((idx) < (int)(count) - 1) { \
        memmove(&(arr)[idx], &(arr)[(idx) + 1], (size_t)((int)(count) - 1 - (idx)) * sizeof(typeof((arr)[0]))); \
    } \
    (count)--; \
} while (0)

static inline bool checkPin(Vector2 mouse, Vector2 pt, PinHit cand, PinHit *out) {
    if (!CheckCollisionPointCircle(mouse, pt, 10.0f)) return false;
    *out = cand;
    out->pos = pt;
    return true;
}

PinHit findPin(Vector2 mouse, const Circuit *c, const Layout *pos) {
    PinHit res = pinhit_empty();

    for (int i = 0; i < c->gatesCount; ++i) {
        float x = pos->gates[i].x;
        float y = pos->gates[i].y;

        if (checkPin(mouse, (Vector2){x + 60, y + 20}, (PinHit){.idx = i, .type = GATE, .isOut = true}, &res)) return res;

        if (isSingleInput(&c->gates[i])) {
            if (checkPin(mouse, (Vector2){x, y + 20}, (PinHit){.idx = i, .type = GATE}, &res)) return res;
        } else {
            if (checkPin(mouse, (Vector2){x, y + 10}, (PinHit){.idx = i, .type = GATE}, &res)) return res;
            if (checkPin(mouse, (Vector2){x, y + 30}, (PinHit){.idx = i, .type = GATE, .gatePin = true}, &res)) return res;
        }
    }

    for (int i = 0; i < c->switchesCount; ++i) {
        Vector2 pt = { pos->switches[i].x + 40, pos->switches[i].y + 20 };
        if (checkPin(mouse, pt, (PinHit){.idx = i, .type = SWITCH, .isOut = true}, &res)) return res;
    }

    for (int i = 0; i < c->ledsCount; ++i) {
        Vector2 pt = { pos->leds[i].x, pos->leds[i].y + 20 };
        if (checkPin(mouse, pt, (PinHit){.idx = i, .type = LED}, &res)) return res;
    }

    return pinhit_empty();
}

bool isMouseOverWire(Vector2 mouse, Vector2 start, Vector2 end) {
    float off = fmaxf(fabsf(end.x - start.x) * 0.5f, 40.0f);
    Vector2 c1 = { start.x + off, start.y };
    Vector2 c2 = { end.x - off, end.y };
    Vector2 prev = start;

    for (int s = 1; s <= 16; ++s) {
        Vector2 curr = GetSplinePointBezierCubic(start, end, c1, c2, (float)s / 16.0f);
        if (CheckCollisionPointLine(mouse, prev, curr, 8.0f)) return true;
        prev = curr;
    }
    return false;
}

static inline bool updateWireEndpoint(Kind k, int *id, Kind targetType, int targetIdx) {
    if (k == targetType && *id > targetIdx) (*id)--;
    return k == targetType && *id == targetIdx;
}

void deleteNode(Circuit *c, Layout *pos, Kind type, int idx) {
    for (int i = (int)c->wiresCount - 1; i >= 0; --i) {
        bool hitFrom = updateWireEndpoint(c->wires[i].fromType, &c->wires[i].from, type, idx);
        bool hitTo = updateWireEndpoint(c->wires[i].toType, &c->wires[i].to, type, idx);
        if (hitFrom || hitTo) {
            ARRAY_REMOVE_AT(c->wires, c->wiresCount, i);
        }
    }

    if (type == GATE) {
        uint16_t cnt = c->gatesCount;
        ARRAY_REMOVE_AT(c->gates, c->gatesCount, idx);
        ARRAY_REMOVE_AT(pos->gates, cnt, idx);
    } else if (type == SWITCH) {
        uint16_t cnt = c->switchesCount;
        ARRAY_REMOVE_AT(c->switches, c->switchesCount, idx);
        ARRAY_REMOVE_AT(pos->switches, cnt, idx);
    } else if (type == LED) {
        uint16_t cnt = c->ledsCount;
        ARRAY_REMOVE_AT(c->leds, c->ledsCount, idx);
        ARRAY_REMOVE_AT(pos->leds, cnt, idx);
    }
}

