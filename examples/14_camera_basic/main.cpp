// Demonstrates the two E1 additions together: Camera2D (world-space
// rendering + coordinate conversion) and the asset-root workflow
// (SetAssetRoot + the CMake copy-assets-beside-the-executable pattern in
// this example's CMakeLists.txt). Camera movement here is plain
// player-controlled panning -- following/smoothing/bounds are deliberately
// left to game code, not part of the engine primitive.

#include "engine/Engine.hpp"

#include <algorithm>
#include <string>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "14 - Camera Basic"});

    // "assets" is resolved against the executable's own directory, not the
    // process's current working directory -- so this works the same
    // whether launched from Visual Studio, PowerShell, or the .exe
    // directly. CMakeLists.txt copies this example's assets/ next to the
    // built executable so "assets" actually resolves to something.
    app.SetAssetRoot("assets");
    const engine::TextureHandle sprite = app.LoadTexture("sprite.png");
    const float spriteWidth = static_cast<float>(app.TextureWidth(sprite));

    // A world wider than the window, so panning the camera reveals
    // something -- the horizontally-scrolling scenario this primitive
    // was added for.
    constexpr float worldWidth = 1600.0f;
    constexpr float worldHeight = 450.0f;
    constexpr float spriteSpacing = 150.0f;

    // A fixed point of interest used below to demonstrate WorldToScreen.
    constexpr engine::Vec2 landmark{800.0f, 120.0f};

    engine::Camera2D camera{.position = {worldWidth / 2.0f, worldHeight / 2.0f}, .zoom = 1.0f};

    constexpr float panSpeed = 300.0f; // world units per second
    constexpr float zoomSpeed = 0.6f;  // zoom factor per second
    constexpr float minZoom = 0.5f;
    constexpr float maxZoom = 2.0f;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        if (app.IsKeyDown(engine::Key::Left) || app.IsKeyDown(engine::Key::A)) {
            camera.position.x -= panSpeed * dt;
        }
        if (app.IsKeyDown(engine::Key::Right) || app.IsKeyDown(engine::Key::D)) {
            camera.position.x += panSpeed * dt;
        }
        if (app.IsKeyDown(engine::Key::Up) || app.IsKeyDown(engine::Key::W)) {
            camera.position.y -= panSpeed * dt;
        }
        if (app.IsKeyDown(engine::Key::Down) || app.IsKeyDown(engine::Key::S)) {
            camera.position.y += panSpeed * dt;
        }
        if (app.IsKeyDown(engine::Key::Q)) {
            camera.zoom = std::max(minZoom, camera.zoom - zoomSpeed * dt);
        }
        if (app.IsKeyDown(engine::Key::E)) {
            camera.zoom = std::min(maxZoom, camera.zoom + zoomSpeed * dt);
        }

        // Screen-to-world and world-to-screen conversion, independent of
        // BeginCameraMode/EndCameraMode below.
        const engine::Vec2 mouseWorld = app.ScreenToWorld(camera, engine::Vec2{app.MouseX(), app.MouseY()});
        const engine::Vec2 landmarkScreen = app.WorldToScreen(camera, landmark);

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.BeginCameraMode(camera);

        // Vertical grid lines purely so panning/zooming stays visible even
        // between sprites.
        for (float x = 0.0f; x <= worldWidth; x += spriteSpacing) {
            app.DrawLine(x, 0.0f, x, worldHeight, engine::Color{225, 225, 225, 255});
        }

        for (float x = 0.0f; x <= worldWidth - spriteWidth; x += spriteSpacing) {
            app.DrawSprite(sprite, x, worldHeight / 2.0f - spriteWidth / 2.0f);
        }

        app.DrawRectangle(landmark.x - 10.0f, landmark.y - 10.0f, 20.0f, 20.0f, engine::Color{200, 60, 60, 255});

        app.EndCameraMode();

        // Screen space from here on -- doesn't pan or zoom with the world above.
        if (landmarkScreen.x >= 0.0f && landmarkScreen.x <= windowWidth && landmarkScreen.y >= 0.0f &&
            landmarkScreen.y <= windowHeight) {
            app.DrawText("landmark", static_cast<int>(landmarkScreen.x) + 12, static_cast<int>(landmarkScreen.y) - 8,
                         16, engine::Color{200, 60, 60, 255});
        }

        app.DrawText("WASD/arrows: pan camera   Q/E: zoom   red square: fixed world landmark", 10, 10, 16,
                     engine::colors::DarkGray);
        app.DrawText("Mouse world position: " + engine::ToString(mouseWorld.x, 1) + ", " +
                         engine::ToString(mouseWorld.y, 1),
                     10, 32, 16, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
