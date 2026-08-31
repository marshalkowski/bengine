#include "engine/Engine.hpp"

#include <string>

// Demonstrates the engine's minimal sound-effect API:
//
//     Engine::LoadSound(path) -> SoundHandle   -- load once, cached by path
//     Engine::PlaySound(handle)                -- trigger playback, fire-and-forget
//
// Audio device initialization/shutdown is handled entirely inside Engine's
// constructor/destructor -- this example never touches that, the same way
// it never sets up the window manually. There is no stop, volume, looping,
// or instance-tracking API: PlaySound just triggers the sound and returns.

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "13 - Audio Basic"});

    const std::string soundPath = std::string(AUDIO_BASIC_ASSET_DIR) + "/beep.wav";
    const engine::SoundHandle beep = app.LoadSound(soundPath.c_str());

    int playCount = 0;

    while (!app.ShouldClose()) {
        if (app.IsKeyPressed(engine::Key::Space)) {
            app.PlaySound(beep);
            ++playCount;
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawText("Press Space to play sound", 20, 20, 20, engine::colors::DarkGray);
        app.DrawText("Played: " + std::to_string(playCount), 20, 60, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
