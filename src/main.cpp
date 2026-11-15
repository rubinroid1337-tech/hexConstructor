#include <raylib.h>
#include <raymath.h>

#include "gate.hpp"
#include "gate-render.hpp"
#include "ware-render.hpp"
#include "gate-system.hpp"

#include <iostream>

int main() {
    constexpr int screen_width  = 800;
    constexpr int screen_height = 450;
    InitWindow(screen_width, screen_height, "hex constructor");

    SetTargetFPS(60);

    Gate gate;
    gate = 
    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        

        EndDrawing();
    }

    CloseWindow();
}
