#include "engine/Engine.hpp"

#include <cstdio>

int main() {
    engine::Engine app({.width = 800, .height = 450, .title = "02 - Time"});

    float elapsedSeconds = 0.0f;

    while (!app.ShouldClose()) {
        const float deltaTime = app.DeltaTime();
        elapsedSeconds += deltaTime;

        app.BeginFrame();
        app.Clear(engine::colors::White);

        char deltaText[64];
        std::snprintf(deltaText, sizeof(deltaText), "Delta time: %.4f s", deltaTime);
        app.DrawText(deltaText, 20, 20, 20, engine::colors::DarkGray);

        char elapsedText[64];
        std::snprintf(elapsedText, sizeof(elapsedText), "Elapsed time: %.2f s", elapsedSeconds);
        app.DrawText(elapsedText, 20, 50, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
