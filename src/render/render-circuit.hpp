#pragma once

#include <cmath>
#include <raylib.h>
#include "../core/circuit.hpp"

inline void DrawWire(Vector2 start, Vector2 end, bool state) {
    float offset = std::max(std::abs(end.x - start.x) * 0.5f, 40.0f);
    Vector2 c1 = { start.x + offset, start.y };
    Vector2 c2 = { end.x - offset, end.y };
    Color col = state ? RED : MAROON;

    DrawSplineBezierCubic(start, c1, c2, end, 6.0f, Fade(BLACK, 0.4f));
    DrawSplineBezierCubic(start, c1, c2, end, 3.5f, col);
}

inline void DrawCircuit(const Circuit& circuit, const std::vector<Vector2>& positions) {
    for (const auto& w : circuit.wires) {
        Vector2 start = { positions[w.from].x + 60, positions[w.from].y + 20 };
        Vector2 end   = { positions[w.to].x,        positions[w.to].y + (w.pin ? 30.0f : 10.0f) };
        DrawWire(start, end, w.state);
    }

    for (size_t i = 0; i < circuit.gates.size(); ++i) {
        Vector2 p = positions[i];
        DrawRectangleRounded({ p.x, p.y, 60, 40 }, 0.2f, 4, DARKGRAY);
        DrawCircle(p.x,      p.y + 10, 4, circuit.gates[i].in[0] ? RED : MAROON);
        DrawCircle(p.x,      p.y + 30, 4, circuit.gates[i].in[1] ? RED : MAROON);
        DrawCircle(p.x + 60, p.y + 20, 4, circuit.gates[i].out   ? RED : MAROON);
    }
}
