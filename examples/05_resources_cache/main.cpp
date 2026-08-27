#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;
    constexpr int spacing = 40;

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "05 - Resources Cache"});

    const std::string spritePath = std::string(RESOURCES_CACHE_ASSET_DIR) + "/sprite.png";

    // Request the same texture path twice. The resource manager should
    // recognize the second request as a duplicate and reuse the
    // already-loaded texture rather than loading the file again.
    engine::TextureHandle first = app.LoadTexture(spritePath.c_str());
    engine::TextureHandle second = app.LoadTexture(spritePath.c_str());

    const std::string loadedCountText = "Loaded textures: " + std::to_string(app.LoadedTextureCount());

    const int firstX = windowWidth / 2 - app.TextureWidth(first) - spacing / 2;
    const int secondX = windowWidth / 2 + spacing / 2;
    const int spriteY = (windowHeight - app.TextureHeight(first)) / 2;

    while (!app.ShouldClose()) {
        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawSprite(first, firstX, spriteY);
        app.DrawSprite(second, secondX, spriteY);

        app.DrawText(loadedCountText, 20, 20, 20, engine::colors::DarkGray);
        app.DrawText("(requested twice, loaded once)", 20, 50, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
