#include "engine/Engine.hpp"

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;
    constexpr float speed = 250.0f; // pixels per second
    constexpr float movingSize = 100.0f;

    constexpr engine::Rect stationary{450.0f, 150.0f, 150.0f, 150.0f};

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "08 - Geometry"});

    float x = 80.0f;
    float y = 180.0f;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        if (app.IsKeyDown(engine::Key::Left) || app.IsKeyDown(engine::Key::A)) {
            x -= speed * dt;
        }
        if (app.IsKeyDown(engine::Key::Right) || app.IsKeyDown(engine::Key::D)) {
            x += speed * dt;
        }
        if (app.IsKeyDown(engine::Key::Up) || app.IsKeyDown(engine::Key::W)) {
            y -= speed * dt;
        }
        if (app.IsKeyDown(engine::Key::Down) || app.IsKeyDown(engine::Key::S)) {
            y += speed * dt;
        }

        const engine::Rect moving{x, y, movingSize, movingSize};
        const bool overlapping = engine::Intersects(moving, stationary);

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawRectangle(stationary.x, stationary.y, stationary.width, stationary.height,
                           engine::Color{70, 110, 180, 255});

        const engine::Color movingColor =
            overlapping ? engine::Color{200, 60, 60, 255} : engine::Color{60, 140, 90, 255};
        app.DrawRectangle(x, y, movingSize, movingSize, movingColor);

        app.DrawText("Move the rectangle with WASD/Arrows", 10, 4, 18, engine::colors::DarkGray);
        app.DrawText(overlapping ? "Intersects: yes" : "Intersects: no", 10, 24, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
