#include "engine/Engine.hpp"

#include "raylib.h"

int main() {
    engine::Engine app({.width = 800, .height = 450, .title = "00 - Hello Engine"});

    while (!app.ShouldClose()) {
        app.BeginFrame();

        ClearBackground(RAYWHITE);
        DrawText("Hello, Engine!", 260, 200, 30, DARKGRAY);

        app.EndFrame();
    }

    return 0;
}
