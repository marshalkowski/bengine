#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;
    constexpr float speed = 200.0f; // pixels per second

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "03 - Input"});

    const std::string spritePath = std::string(INPUT_ASSET_DIR) + "/sprite.png";
    engine::TextureHandle sprite = app.LoadTexture(spritePath.c_str());

    float x = (windowWidth - app.TextureWidth(sprite)) / 2.0f;
    float y = (windowHeight - app.TextureHeight(sprite)) / 2.0f;

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

        app.BeginFrame();
        app.Clear(engine::colors::White);
        app.DrawSprite(sprite, static_cast<int>(x), static_cast<int>(y));
        app.EndFrame();
    }

    return 0;
}
