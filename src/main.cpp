#include "raylib.h"
#include <string>

#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/switch.hpp"
#include "core/led.hpp"
#include "core/save.hpp"
#include "render/circuit-render.hpp"
#include "input/editor.hpp"
#include "input/context-menu.hpp"

enum class AppState { MENU, EDITOR };

bool isValidChar(int key) {
    return (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') ||
           (key >= '0' && key <= '9') || key == '_' || key == '-' || key == ' ';
}

int main() {
    InitWindow(800, 450, "hex constructor");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    AppState state = AppState::MENU;
    Circuit circuit;
    Layout layout;
    Editor editor;
    SaveManager saveManager("saves", 10.0f);
    ContextMenu contextMenu;

    // Вспомогательная функция для получения мировых координат клика
    auto getSpawnPos = [&]() -> Vector2 {
        return GetScreenToWorld2D(contextMenu.getOpenPosition(), editor.camera);
    };

    // Вентили (без NAND)
    constexpr GateType availableGates[] = {
        GateType::XOR,
        GateType::OR,
        GateType::AND,
        GateType::NOT
    };

    for (GateType type : availableGates) {
        contextMenu.addButton(Gate{ .type = type }.label(), [&, type]() {
            circuit.gates.push_back(Gate{ .type = type });
            layout.gates.push_back(getSpawnPos());
        });
    }

    // Разделитель между вентилями и элементами ввода/вывода
    contextMenu.addSeparator();

    // Switch и Led
    contextMenu.addButton("SWITCH", [&]() {
        circuit.switches.push_back(Switch{ .on = false });
        layout.switches.push_back(getSpawnPos());
    });

    contextMenu.addButton("LED", [&]() {
        circuit.leds.push_back(Led{ .on = false });
        layout.leds.push_back(getSpawnPos());
    });

    std::string inputName;

    while (!WindowShouldClose()) {
        // ==================== ОБНОВЛЕНИЕ ЛОГИКИ ====================
        if (state == AppState::MENU) {
            int key = GetCharPressed();
            while (key > 0) {
                if (isValidChar(key) && inputName.size() < 28) {
                    inputName.push_back(static_cast<char>(key));
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !inputName.empty()) {
                inputName.pop_back();
            }
            if (IsKeyPressed(KEY_ENTER) && saveManager.createProject(inputName, circuit, layout)) {
                editor = Editor{};
                inputName.clear();
                state = AppState::EDITOR;
            }
        } 
        else if (state == AppState::EDITOR) {
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                contextMenu.open(GetMousePosition());
            }

            contextMenu.update();

            if (!contextMenu.isVisible()) {
                editor.hotkeysUpdate(circuit, layout);

                if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
                    saveManager.saveCurrent(circuit, layout);
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    contextMenu.close();
                    saveManager.closeProject(circuit, layout);
                    state = AppState::MENU;
                }
            }

            circuit.tick();
            saveManager.updateAutoSave(GetFrameTime(), circuit, layout);
        }

        // ==================== ОТРИСОВКА ====================
        BeginDrawing();
        ClearBackground({ 30, 32, 38, 255 });

        if (state == AppState::MENU) {
            DrawText("HEX CONSTRUCTOR", 40, 25, 24, RAYWHITE);
            DrawText("Select project or create new:", 40, 58, 16, LIGHTGRAY);

            Rectangle inputBox = { 40.0f, 90.0f, 360.0f, 36.0f };
            DrawRectangleRec(inputBox, { 45, 48, 56, 255 });
            DrawRectangleLinesEx(inputBox, 2, SKYBLUE);
            DrawText(inputName.empty() ? "New project name..." : inputName.c_str(),
                     50, 100, 18, inputName.empty() ? GRAY : RAYWHITE);

            Rectangle createBtn = { 410.0f, 90.0f, 110.0f, 36.0f };
            bool hoverCreate = CheckCollisionPointRec(GetMousePosition(), createBtn);
            DrawRectangleRec(createBtn, hoverCreate ? Color{ 60, 170, 90, 255 } : Color{ 45, 135, 70, 255 });
            DrawText("Create", 435, 100, 18, WHITE);

            if (hoverCreate && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                saveManager.createProject(inputName, circuit, layout)) {
                editor = Editor{};
                inputName.clear();
                state = AppState::EDITOR;
            }

            int y = 150;
            for (const auto& name : saveManager.list()) {
                Rectangle itemRect = { 40.0f, static_cast<float>(y), 430.0f, 36.0f };
                Rectangle delRect  = { 480.0f, static_cast<float>(y), 40.0f, 36.0f };

                bool hoverItem = CheckCollisionPointRec(GetMousePosition(), itemRect);
                bool hoverDel  = CheckCollisionPointRec(GetMousePosition(), delRect);

                DrawRectangleRec(itemRect, hoverItem ? Color{ 65, 72, 85, 255 } : Color{ 45, 48, 56, 255 });
                DrawText(name.c_str(), 52, y + 9, 18, RAYWHITE);

                DrawRectangleRec(delRect, hoverDel ? Color{ 200, 60, 60, 255 } : Color{ 140, 45, 45, 255 });
                DrawText("X", 494, y + 9, 18, WHITE);

                if (hoverItem && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                    saveManager.openProject(name, circuit, layout)) {
                    editor = Editor{};
                    state = AppState::EDITOR;
                }

                if (hoverDel && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    saveManager.deleteProject(name);
                    break;
                }

                y += 44;
            }
        } 
        else if (state == AppState::EDITOR) {
            BeginMode2D(editor.camera);
                DrawCircuit(circuit, layout, editor.selection);
                editor.draw(circuit, layout);
            EndMode2D();

            contextMenu.draw();

            DrawText(("Project: " + saveManager.activeName() + "  [ESC: Menu]").c_str(), 12, 12, 16, LIGHTGRAY);
        }

        EndDrawing();
    }

    saveManager.saveCurrent(circuit, layout);
    CloseWindow();
    return 0;
}

