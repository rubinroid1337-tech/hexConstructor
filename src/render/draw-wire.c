#include "draw-wire.h"

void DrawWire(Vector2 start, Vector2 end, bool on) {
    float offset = fmaxf(fabsf(end.x - start.x) * 0.5f, 40.0f);
    Vector2 pts[4] = {
        start,
        { start.x + offset, start.y },
        { end.x - offset, end.y },
        end
    };
    Color col = on ? RED : DARK_RED;

    DrawSplineBezierCubic(pts, 4, 6.0f, Fade(BLACK, 0.5f));
    DrawSplineBezierCubic(pts, 4, 3.5f, col);
}
