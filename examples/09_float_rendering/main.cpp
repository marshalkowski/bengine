#include "engine/Engine.hpp"

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;
    constexpr float speed = 150.0f; // pixels per second
    constexpr float size = 60.0f;

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "09 - Float Rendering"});

    // Position lives as float the entire time — nothing here ever converts
    // it to int. DrawRectangle takes floats directly.
    float x = 0.0f;
    const float y = (windowHeight - size) / 2.0f;
    float velocityX = speed;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        x += velocityX * dt;
        if (x + size > windowWidth) {
            x = windowWidth - size;
            velocityX = -speed;
        } else if (x < 0.0f) {
            x = 0.0f;
            velocityX = speed;
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawRectangle(x, y, size, size, engine::Color{60, 140, 90, 255});

        app.DrawText("Bouncing rectangle drawn straight from a float position (no casts)",
                     10, 10, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
