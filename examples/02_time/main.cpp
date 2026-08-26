#include "engine/Engine.hpp"

int main() {
    engine::Engine app({.width = 800, .height = 450, .title = "02 - Time"});

    float elapsedSeconds = 0.0f;

    while (!app.ShouldClose()) {
        const float deltaTime = app.DeltaTime();
        elapsedSeconds += deltaTime;

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawText("Delta time: " + engine::ToString(deltaTime, 4) + " s", 20, 20, 20, engine::colors::DarkGray);
        app.DrawText("Elapsed time: " + engine::ToString(elapsedSeconds, 2) + " s", 20, 50, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
