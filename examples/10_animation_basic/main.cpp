#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    constexpr engine::AnimationClip clip{
        .firstFrame = 0,
        .frameCount = 4,
        .frameWidth = 64.0f,
        .frameHeight = 64.0f,
        .frameDuration = 0.15f,
        .loop = true,
    };

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "10 - Animation Basic"});

    const std::string assetDir = ANIMATION_BASIC_ASSET_DIR;
    const engine::TextureHandle spriteSheet = app.LoadTexture((assetDir + "/spritesheet.png").c_str());

    engine::Animation animation(clip);

    const float x = (windowWidth - clip.frameWidth) / 2.0f;
    const float y = (windowHeight - clip.frameHeight) / 2.0f;

    while (!app.ShouldClose()) {
        animation.Update(app.DeltaTime());

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawSpriteRegion(spriteSheet, animation.CurrentFrameRect(), x, y);

        app.DrawText("Looping sprite-sheet animation (4 frames, 0.15s each)", 10, 10, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
