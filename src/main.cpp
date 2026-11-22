#include "raylib.h"
#include "core/circuit.hpp"
#include "render/circuit-render.hpp"
#include "input/editor.hpp"
#include "core/save.hpp"

int main() {
    InitWindow(800, 450, "hex constructor");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Circuit circuit;
    Layout layout;
    Editor editor;

    loadFile(circuit, layout, "save.json");

    float autoSaveTimer = 0.0f;
    const float autoSaveInterval = 10.0f;

    while (!WindowShouldClose()) {
        editor.hotkeysUpdate(circuit, layout);
        circuit.tick();

        autoSaveTimer += GetFrameTime();
        if (autoSaveTimer >= autoSaveInterval) {
            autoSaveTimer = 0.0f;
            saveFile(circuit, layout, "save.json");
        }

        BeginDrawing();
        ClearBackground({ 30, 32, 38, 255 });

        BeginMode2D(editor.camera);
            DrawCircuit(circuit, layout, editor.selection);
            editor.draw(circuit, layout);
        EndMode2D();

        EndDrawing();
    }

    saveFile(circuit, layout, "save.json");

    CloseWindow();
}
