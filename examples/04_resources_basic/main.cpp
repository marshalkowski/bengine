#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "04 - Resources Basic"});

    // Request the texture resource by filepath. The returned handle is
    // engine-owned and opaque: this code never sees how or where the
    // texture is actually stored, only that Engine can draw it.
    const std::string spritePath = std::string(RESOURCES_BASIC_ASSET_DIR) + "/sprite.png";
    engine::TextureHandle sprite = app.LoadTexture(spritePath.c_str());

    const int spriteX = (windowWidth - app.TextureWidth(sprite)) / 2;
    const int spriteY = (windowHeight - app.TextureHeight(sprite)) / 2;

    while (!app.ShouldClose()) {
        app.BeginFrame();
        app.Clear(engine::colors::White);
        app.DrawSprite(sprite, spriteX, spriteY);
        app.EndFrame();
    }

    return 0;
}
