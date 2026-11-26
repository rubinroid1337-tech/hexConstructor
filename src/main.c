#include <stdlib.h>
#include <raylib.h>

#include "core/circuit.h"
#include "render/layout.h"
#include "render/circuit-render.h"
#include "input/editor.h"
#include "input/hotkeys-system.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "hexConstructor");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Circuit *circuit = calloc(1, sizeof(Circuit));
    Layout *layout   = calloc(1, sizeof(Layout));
    Editor *editor   = calloc(1, sizeof(Editor));

    if (!circuit || !layout || !editor) {
        CloseWindow();
        return 1;
    }

    editor_init(editor);

    bool shouldQuit = false;
    while (!WindowShouldClose() && !shouldQuit) {
        shouldQuit = hotkeys_update(editor, circuit, layout);
        tick(circuit);

        BeginDrawing();
        ClearBackground((Color){ 24, 24, 28, 255 });

        BeginMode2D(editor->camera);
            DrawCircuit(circuit, layout, &editor->selection);
            editor_draw(editor, circuit, layout);
        EndMode2D();

        DrawFPS(10, 10);
        EndDrawing();
    }

    editor_free(editor);
    free(editor);
    free(layout);
    free(circuit);

    CloseWindow();
    return 0;
}

