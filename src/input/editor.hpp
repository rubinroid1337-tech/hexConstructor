#pragma once
#include <vector>
#include <raylib.h>
#include "../core/circuit.hpp"

struct Editor {
    int draggedGate = -1;
    Vector2 dragOffset{};

    void update(std::vector<Vector2>& positions) {
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = (int)positions.size() - 1; i >= 0; --i) {
                Rectangle box = { positions[i].x, positions[i].y, 60.0f, 40.0f };
                if (CheckCollisionPointRec(mouse, box)) {
                    draggedGate = i;
                    dragOffset = { mouse.x - positions[i].x, mouse.y - positions[i].y };
                    break;
                }
            }
        }

        if (draggedGate != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            positions[draggedGate] = { mouse.x - dragOffset.x, mouse.y - dragOffset.y };
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            draggedGate = -1;
        }
    }
};
