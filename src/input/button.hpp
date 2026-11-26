#pragma once

#include <raylib.h>
#include <functional>
#include <string>

struct Button {
    std::string name;
    Rectangle bounds{};
    std::function<void()> onClick;

    void update() {
        if (CheckCollisionPointRec(GetMousePosition(), bounds) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (onClick) {
                onClick();
            }
        }
    }

    void draw() const {
        bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

        DrawRectangleRec(bounds, hovered ? Color{ 65, 72, 85, 255 } : Color{ 45, 48, 56, 255 });
        DrawText(name.c_str(), static_cast<int>(bounds.x) + 12, static_cast<int>(bounds.y) + 8, 16, RAYWHITE);
    }
};

