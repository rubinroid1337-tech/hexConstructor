#include <raylib.h>
#include "core/circuit.h"
#include "render/layout.h"
#include "render/selection.h"
#include "render/circuit-render.h"

int main(void) {
    static Circuit c = {0};
    static Layout pos = {0};
    static Selection sel = {0};

    int i0 = 0, i1 = 1, i2 = 2;

    c.switchesCount = 2;
    pos.switches[i0] = (Vector2){ 120, 150 };
    pos.switches[i1] = (Vector2){ 120, 270 };

    c.gates[i0] = (Gate){ .type = GATE_AND };
    c.gatesCount = 1;
    pos.gates[i0] = (Vector2){ 320, 210 };
    sel.gates[i0] = true;
    sel.count = 1;

    c.ledsCount = 1;
    pos.leds[i0] = (Vector2){ 540, 210 };

    c.wires[i0] = (Wire){ .fromType = SWITCH, .from = 0, .toType = GATE, .to = 0, .pin = 0 };
    c.wires[i1] = (Wire){ .fromType = SWITCH, .from = 1, .toType = GATE, .to = 0, .pin = 1 };
    c.wires[i2] = (Wire){ .fromType = GATE,   .from = 0, .toType = LED,  .to = 0, .pin = 0 };
    c.wiresCount = 3;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(720, 460, "hexConstructor");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ONE)) c.switches[i0].on = !c.switches[i0].on;
        if (IsKeyPressed(KEY_TWO)) c.switches[i1].on = !c.switches[i1].on;

        tick(&c);

        BeginDrawing();
        ClearBackground((Color){ 24, 24, 28, 255 });
        DrawText("Press [1] and [2] to toggle switches", 20, 20, 18, GRAY);
        DrawCircuit(&c, &pos, &sel);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

