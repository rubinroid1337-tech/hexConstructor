#include "circuit-render.h"

void DrawCircuit(const Circuit *c, const Layout *pos, const Selection *sel) {
    for (int i = 0; i < c->wiresCount; i++) {
        const Wire *w = &c->wires[i];
        Vector2 start = (w->fromType == SWITCH)
            ? (Vector2){ pos->switches[w->from].x + 40, pos->switches[w->from].y + 20 }
            : (Vector2){ pos->gates[w->from].x + 60,    pos->gates[w->from].y + 20 };

        Vector2 end = {0};
        if (w->toType == LED) {
            end = (Vector2){ pos->leds[w->to].x, pos->leds[w->to].y + 20 };
        } else {
            float pinY = isSingleInput(&c->gates[w->to]) ? 20.0f : (w->pin ? 30.0f : 10.0f);
            end = (Vector2){ pos->gates[w->to].x, pos->gates[w->to].y + pinY };
        }

        DrawWire(start, end, w->on);
    }

    for (int i = 0; i < c->gatesCount; i++) {
        const Gate *g = &c->gates[i];
        Vector2 p = pos->gates[i];
        DrawRectangleRounded((Rectangle){ p.x, p.y, 60, 40 }, 0.2f, 4, DARKGRAY);
        if (selection_has(sel, GATE, i)) {
            DrawRectangleRoundedLinesEx((Rectangle){ p.x - 2, p.y - 2, 64, 44 }, 0.2f, 4, 2.0f, SKYBLUE);
        }

        const char *name = label(g);
        int textWidth = MeasureText(name, 14);
        DrawText(name, (int)p.x + (60 - textWidth) / 2, (int)p.y + 13, 14, RAYWHITE);

        if (isSingleInput(g)) {
            DrawCircle((int)p.x, (int)p.y + 20, 4, g->in[0] ? RED : DARK_RED);
        } else {
            DrawCircle((int)p.x, (int)p.y + 10, 4, g->in[0] ? RED : DARK_RED);
            DrawCircle((int)p.x, (int)p.y + 30, 4, g->in[1] ? RED : DARK_RED);
        }
        DrawCircle((int)p.x + 60, (int)p.y + 20, 4, g->out ? RED : DARK_RED);
    }

    for (int i = 0; i < c->switchesCount; i++) {
        Vector2 p = pos->switches[i];
        Color col = c->switches[i].on ? RED : DARK_RED;
        DrawRectangleRounded((Rectangle){ p.x, p.y, 40, 40 }, 0.2f, 4, DARKGRAY);
        if (selection_has(sel, SWITCH, i)) {
            DrawRectangleRoundedLinesEx((Rectangle){ p.x - 2, p.y - 2, 44, 44 }, 0.2f, 4, 2.0f, SKYBLUE);
        }

        DrawRectangleRounded((Rectangle){ p.x + 8, p.y + 8, 24, 24 }, 0.3f, 4, col);
        DrawCircle((int)p.x + 40, (int)p.y + 20, 4, col);
    }

    for (int i = 0; i < c->ledsCount; i++) {
        Vector2 p = pos->leds[i];
        Color col = c->leds[i].on ? RED : DARK_RED;
        DrawRectangleRounded((Rectangle){ p.x, p.y, 40, 40 }, 0.5f, 8, DARKGRAY);
        if (selection_has(sel, LED, i)) {
            DrawRectangleRoundedLinesEx((Rectangle){ p.x - 2, p.y - 2, 44, 44 }, 0.5f, 8, 2.0f, SKYBLUE);
        }

        DrawCircle((int)p.x + 20, (int)p.y + 20, 13, col);
        DrawCircle((int)p.x,      (int)p.y + 20, 4,  col);
    }
}
