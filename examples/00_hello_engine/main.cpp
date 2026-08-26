#include "engine/Engine.hpp"

int main() {
    engine::Engine app({.width = 800, .height = 450, .title = "00 - Hello Engine"});

    while (!app.ShouldClose()) {
        app.BeginFrame();

        app.Clear(engine::colors::White);
        app.DrawText("Hello, Engine!", 260, 200, 30, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
