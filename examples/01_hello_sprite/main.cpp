#include "engine/Engine.hpp"

#include <string>

int main() {
    engine::Engine app({.width = 800, .height = 450, .title = "01 - Hello Sprite"});

    const std::string spritePath = std::string(HELLO_SPRITE_ASSET_DIR) + "/sprite.png";
    engine::TextureHandle sprite = app.LoadTexture(spritePath.c_str());

    const int spriteX = (800 - app.TextureWidth(sprite)) / 2;
    const int spriteY = (450 - app.TextureHeight(sprite)) / 2;

    while (!app.ShouldClose()) {
        app.BeginFrame();

        app.Clear(engine::colors::White);
        app.DrawSprite(sprite, spriteX, spriteY);

        app.EndFrame();
    }

    return 0;
}
