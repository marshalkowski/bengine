#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    constexpr float frameWidth = 64.0f;
    constexpr float frameHeight = 64.0f;
    constexpr int frameCount = 4;
    constexpr float frameDuration = 0.15f; // seconds per frame

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "10 - Animation Basic"});

    const std::string assetDir = ANIMATION_BASIC_ASSET_DIR;
    const engine::TextureHandle spriteSheet = app.LoadTexture((assetDir + "/spritesheet.png").c_str());

    const float x = (windowWidth - frameWidth) / 2.0f;
    const float y = (windowHeight - frameHeight) / 2.0f;

    int currentFrame = 0;
    float accumulatedTime = 0.0f;

    while (!app.ShouldClose()) {
        // Time-based frame advancement: correct regardless of render frame
        // rate, and handles a stall spanning multiple frame durations.
        accumulatedTime += app.DeltaTime();
        while (accumulatedTime >= frameDuration) {
            accumulatedTime -= frameDuration;
            currentFrame = (currentFrame + 1) % frameCount;
        }

        const engine::Rect sourceRect{currentFrame * frameWidth, 0.0f, frameWidth, frameHeight};

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawSpriteRegion(spriteSheet, sourceRect, x, y);

        app.DrawText("Looping sprite-sheet animation (4 frames, 0.15s each)", 10, 10, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
