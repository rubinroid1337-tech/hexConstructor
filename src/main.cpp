#include "raylib.h"
#include "core/circuit.hpp"
#include "render/circuit-render.hpp"
#include "input/editor.hpp"
#include "core/save.hpp"

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

    loadFile(circuit, layout, "save.json");

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

    saveFile(circuit, layout, "save.json");

    CloseWindow();
    return 0;
}
