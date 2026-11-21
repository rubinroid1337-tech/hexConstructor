#pragma once
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "../core/circuit.hpp"
#include "../core/kind.hpp"
#include "layout.hpp"
#include "selection.hpp"
#include "draw-wire.hpp"

inline void DrawCircuit(const Circuit& c, const Layout& pos, const Selection& sel) {
    for (const auto& w : c.wires) {
        Vector2 start = (w.fromType == SWITCH)
            ? Vector2{ pos.switches[w.from].x + 40, pos.switches[w.from].y + 20 }
            : Vector2{ pos.gates[w.from].x + 60,    pos.gates[w.from].y + 20 };

        Vector2 end{};
        if (w.toType == LED) {
            end = { pos.leds[w.to].x, pos.leds[w.to].y + 20 };
        } else {
            float pinY = c.gates[w.to].isSingleInput() ? 20.0f : (w.pin ? 30.0f : 10.0f);
            end = { pos.gates[w.to].x, pos.gates[w.to].y + pinY };
        }

        DrawWire(start, end, w.on);
    }

    for (int i = 0; i < (int)c.gates.size(); ++i) {
        Vector2 p = pos.gates[i];
        DrawRectangleRounded({ p.x, p.y, 60, 40 }, 0.2f, 4, DARKGRAY);
        if (sel.has(GATE, i)) DrawRectangleRoundedLinesEx({ p.x - 2, p.y - 2, 64, 44 }, 0.2f, 4, 2.0f, SKYBLUE);

        const char* name = c.gates[i].label();
        int textWidth = MeasureText(name, 14);
        DrawText(name, p.x + (60 - textWidth) / 2, p.y + 13, 14, RAYWHITE);

        if (c.gates[i].isSingleInput()) {
            DrawCircle(p.x, p.y + 20, 4, c.gates[i].in[0] ? RED : DARK_RED);
        } else {
            DrawCircle(p.x, p.y + 10, 4, c.gates[i].in[0] ? RED : DARK_RED);
            DrawCircle(p.x, p.y + 30, 4, c.gates[i].in[1] ? RED : DARK_RED);
        }
        DrawCircle(p.x + 60, p.y + 20, 4, c.gates[i].out ? RED : DARK_RED);
    }

    for (int i = 0; i < (int)c.switches.size(); ++i) {
        Vector2 p = pos.switches[i];
        DrawRectangleRounded({ p.x, p.y, 40, 40 }, 0.2f, 4, DARKGRAY);
        if (sel.has(SWITCH, i)) DrawRectangleRoundedLinesEx({ p.x - 2, p.y - 2, 44, 44 }, 0.2f, 4, 2.0f, SKYBLUE);

        DrawRectangleRounded({ p.x + 8, p.y + 8, 24, 24 }, 0.3f, 4, c.switches[i].on ? RED : DARK_RED);
        DrawCircle(p.x + 40, p.y + 20, 4, c.switches[i].on ? RED : DARK_RED);
    }

    for (int i = 0; i < (int)c.leds.size(); ++i) {
        Vector2 p = pos.leds[i];
        DrawRectangleRounded({ p.x, p.y, 40, 40 }, 0.5f, 8, DARKGRAY);
        if (sel.has(LED, i)) DrawRectangleRoundedLinesEx({ p.x - 2, p.y - 2, 44, 44 }, 0.5f, 8, 2.0f, SKYBLUE);

        DrawCircle(p.x + 20, p.y + 20, 13, c.leds[i].on ? RED : DARK_RED);
        DrawCircle(p.x,      p.y + 20, 4,  c.leds[i].on ? RED : DARK_RED);
    }
}
