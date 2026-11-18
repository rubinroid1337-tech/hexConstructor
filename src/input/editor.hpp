#pragma once
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "raymath.h"
#include "../core/circuit.hpp"
#include "../render/circuit-render.hpp"
#include "../core/kind.hpp"
#include "pin-hit.hpp"
#include "clipboard.hpp"

struct Editor {
    Camera2D camera{ {0, 0}, {0, 0}, 0.0f, 1.0f };
    Selection selection{};
    Clipboard clipboard{};

    bool isPanning = false;
    Vector2 panLastMouse{};

    bool isSelecting = false;
    Vector2 selectStart{};
    Rectangle selectBox{};

    bool isDragging = false;
    Vector2 dragLastWorld{};
    Vector2 clickStartWorld{};

    PinHit wireStart{};

    PinHit findPin(Vector2 mouse, const Layout& pos) {
        auto hit = [&](Vector2 pt, PinHit res) -> bool {
            if (!CheckCollisionPointCircle(mouse, pt, 10)) return false;
            res.pos = pt;
            return true;
        };

        for (int i = 0; i < (int)pos.gates.size(); ++i) {
            Vector2 p = pos.gates[i];
            if (CheckCollisionPointCircle(mouse, { p.x, p.y + 10 }, 10))
                return { .pos = { p.x, p.y + 10 }, .idx = i };
            if (CheckCollisionPointCircle(mouse, { p.x, p.y + 30 }, 10))
                return { .pos = { p.x, p.y + 30 }, .idx = i, .gatePin = true };
            if (CheckCollisionPointCircle(mouse, { p.x + 60, p.y + 20 }, 10))
                return { .pos = { p.x + 60, p.y + 20 }, .idx = i, .isOut = true };
        }
        for (int i = 0; i < (int)pos.switches.size(); ++i) {
            Vector2 p = { pos.switches[i].x + 40, pos.switches[i].y + 20 };
            if (CheckCollisionPointCircle(mouse, p, 10))
                return { .pos = p, .idx = i, .type = SWITCH, .isOut = true };
        }
        for (int i = 0; i < (int)pos.leds.size(); ++i) {
            Vector2 p = { pos.leds[i].x, pos.leds[i].y + 20 };
            if (CheckCollisionPointCircle(mouse, p, 10))
                return { .pos = p, .idx = i, .type = LED };
        }
        return {};
    }

    bool isMouseOverWire(Vector2 mouse, Vector2 start, Vector2 end) {
        float offset = std::max(std::abs(end.x - start.x) * 0.5f, 40.0f);
        Vector2 c1 = { start.x + offset, start.y };
        Vector2 c2 = { end.x - offset, end.y };
        Vector2 prev = start;
        for (int s = 1; s <= 16; ++s) {
            float t = s / 16.0f, u = 1.0f - t;
            Vector2 curr = {
                u*u*u*start.x + 3*u*u*t*c1.x + 3*u*t*t*c2.x + t*t*t*end.x,
                u*u*u*start.y + 3*u*u*t*c1.y + 3*u*t*t*c2.y + t*t*t*end.y
            };
            if (CheckCollisionPointLine(mouse, prev, curr, 8)) return true;
            prev = curr;
        }
        return false;
    }

    void deleteNode(Circuit& c, Layout& pos, Kind type, int idx) {
        for (int i = (int)c.wires.size() - 1; i >= 0; --i) {
            auto& w = c.wires[i];
            if ((w.fromType == type && w.from == idx) || (w.toType == type && w.to == idx)) {
                c.wires.erase(c.wires.begin() + i);
            } else {
                if (w.fromType == type && w.from > idx) w.from--;
                if (w.toType == type && w.to > idx) w.to--;
            }
        }
        if (type == GATE)        { c.gates.erase(c.gates.begin() + idx);       pos.gates.erase(pos.gates.begin() + idx); }
        else if (type == SWITCH) { c.switches.erase(c.switches.begin() + idx); pos.switches.erase(pos.switches.begin() + idx); }
        else if (type == LED)    { c.leds.erase(c.leds.begin() + idx);         pos.leds.erase(pos.leds.begin() + idx); }
        wireStart = {};
    }

    void draw(const Layout& pos) {
        if (isSelecting) {
            DrawRectangleRec(selectBox, Fade(SKYBLUE, 0.2f));
            DrawRectangleLinesEx(selectBox, 1.5f, SKYBLUE);
        }
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), camera);
        PinHit hover = findPin(m, pos);

        if (hover && (wireStart || IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
            DrawCircleLinesV(hover.pos, 7.0f, RAYWHITE);
        }

        if (!wireStart) return;

        bool canConnect = hover && hover.isOut != wireStart.isOut &&
                          !(hover.type == wireStart.type && hover.idx == wireStart.idx);

        Vector2 target = canConnect ? hover.pos : m;

        if (wireStart.isOut) DrawWire(wireStart.pos, target, true);
        else                    DrawWire(target, wireStart.pos, true);
    }
};

#include "hotkeys-system.hpp"
