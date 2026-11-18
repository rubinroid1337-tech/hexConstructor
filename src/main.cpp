#include "raylib.h"
#include "core/circuit.hpp"
#include "render/circuit-render.hpp"
#include "input/editor.hpp"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 450, "hex constructor");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Circuit circuit;
    Layout layout;
    Hotkeys hotkeys;
    Editor editor;

    circuit.switches.resize(2);
    layout.switches = { { 120, 120 }, { 120, 240 } };

    circuit.gates.resize(1);
    layout.gates = { { 360, 180 } };

    circuit.leds.resize(1);
    layout.leds = { { 600, 180 } };

    circuit.wires = {
        { 0, 0, SWITCH, GATE, false, false }, // Switch 0 -> Gate 0 (вход A)
        { 1, 0, SWITCH, GATE, true,  false }, // Switch 1 -> Gate 0 (вход B)
        { 0, 0, GATE,   LED,  false, false }  // Gate 0   -> Led 0
    };

    while (!WindowShouldClose()) {
        hotkeys.hotkeysUpdate(circuit, layout);
        circuit.tick();

        BeginDrawing();
        ClearBackground({ 30, 32, 38, 255 });

        BeginMode2D(editor.camera);
            DrawCircuit(circuit, layout, editor.selection);
            editor.draw(layout);
        EndMode2D();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
