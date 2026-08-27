#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    constexpr engine::AnimationClip idleClip{
        .firstFrame = 0,
        .frameCount = 4,
        .frameWidth = 64.0f,
        .frameHeight = 64.0f,
        .frameDuration = 0.15f,
        .loop = true,
    };

    constexpr engine::AnimationClip actionClip{
        .firstFrame = 4,
        .frameCount = 3,
        .frameWidth = 64.0f,
        .frameHeight = 64.0f,
        .frameDuration = 0.12f,
        .loop = false,
    };

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "11 - Animation Clip"});

    const std::string assetDir = ANIMATION_CLIP_ASSET_DIR;
    const engine::TextureHandle spriteSheet = app.LoadTexture((assetDir + "/spritesheet.png").c_str());

    // The example itself decides which clip is currently active — the
    // Animation type has no concept of "idle" or "action".
    engine::Animation idleAnimation(idleClip);
    engine::Animation actionAnimation(actionClip);
    bool playingAction = false;

    const float x = (windowWidth - idleClip.frameWidth) / 2.0f;
    const float y = (windowHeight - idleClip.frameHeight) / 2.0f;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        if (app.IsKeyPressed(engine::Key::Up)) {
            playingAction = true;
            actionAnimation.Restart();
        }

        if (playingAction) {
            actionAnimation.Update(dt);
            if (actionAnimation.IsComplete()) {
                playingAction = false;
            }
        } else {
            idleAnimation.Update(dt);
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        const engine::Rect sourceRect =
            playingAction ? actionAnimation.CurrentFrameRect() : idleAnimation.CurrentFrameRect();
        app.DrawSpriteRegion(spriteSheet, sourceRect, x, y);

        app.DrawText("Looping idle animation. Press Up arrow for a one-shot action.",
                     10, 10, 18, engine::colors::DarkGray);
        app.DrawText(playingAction ? "Playing: one-shot action" : "Playing: looping idle",
                     10, 30, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
