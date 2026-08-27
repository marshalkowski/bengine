#include "engine/Engine.hpp"

int main() {
    engine::Engine app({.width = 800, .height = 450, .title = "07 - Primitives"});

    while (!app.ShouldClose()) {
        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawRectangle(60, 60, 200, 120, engine::Color{200, 60, 60, 255});
        app.DrawRectangle(300, 60, 200, 120, engine::Color{60, 140, 90, 255});

        app.DrawLine(60, 220, 500, 220, engine::Color{60, 90, 200, 255});
        app.DrawLine(60, 240, 260, 340, engine::colors::DarkGray);

        app.DrawText("Rectangles and lines drawn through the engine API", 20, 380, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
