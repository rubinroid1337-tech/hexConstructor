#pragma once

#include <raylib.h>
#include <raymath.h>

#include "ware.hpp"

struct WareRender {
    Vector2 start;
    Vector2 end;
    float thickness;
    Color color;

    void wareRender() {
        float distance = std::abs(end.x - start.x);
        float tangentOffset = distance * 0.5f;
        color = wire.inputPin ? RED : MAROON;

        if (tangentOffset < 60.0f) tangentOffset = 60.0f;

        Vector2 control1 = { start.x + tangentOffset, start.y };
        Vector2 control2 = { end.x - tangentOffset, end.y };

        DrawSplineBezierCubic(start, control1, control2, end, thickness + 3.0f, Fade(BLACK, 0.5f));

        DrawSplineBezierCubic(start, control1, control2, end, thickness, color);

        DrawCircleV(start, thickness * 1.4f, DARKGRAY);
        DrawCircleV(start, thickness * 0.9f, color);

        DrawCircleV(end, thickness * 1.4f, DARKGRAY);
        DrawCircleV(end, thickness * 0.9f, color);
    }
};
