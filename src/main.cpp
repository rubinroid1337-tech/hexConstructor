#include "raylib.h"
#include "core/circuit.hpp"
#include "render/circuit-render.hpp"
#include "input/editor.hpp"

int main() {
    InitWindow(800, 450, "Logic Simulator");
    SetTargetFPS(60);

    Circuit circuit;
    std::vector<Vector2> positions;
    Editor editor;

    // Добавим пару вентилей для теста
    circuit.gates.resize(2);
    positions.push_back({ 150, 150 });
    positions.push_back({ 400, 200 });
    circuit.wires.push_back({ 0, 1, false, false });

    while (!WindowShouldClose()) {
        editor.update(positions);  // 1. Обрабатываем перетаскивание мышью
        circuit.tick();            // 2. Считаем логику схемы

        BeginDrawing();
        ClearBackground({ 30, 32, 38, 255 });
        DrawCircuit(circuit, positions); // 3. Рисуем на экране
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
