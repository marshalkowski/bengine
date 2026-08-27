#include "engine/Engine.hpp"

#include <algorithm>
#include <string>

// Everything in this file is game-specific: bounds/overlap, the collectible,
// the hazard and its motion pattern, and score/hit state are all rules of
// this particular prototype, not demonstrated engine capabilities. The
// engine only supplies window lifecycle, input, timing, resources, and
// rendering (including primitives).

namespace {

// Prototype-local axis-aligned bounds + overlap test. The engine has no
// equivalent concept yet — see the post-prototype review for whether this
// use turned out to justify one.
struct Rect {
    float x, y, width, height;
};

bool Intersects(const Rect& a, const Rect& b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}

constexpr int windowWidth = 800;
constexpr int windowHeight = 450;
constexpr int hudHeight = 44;

constexpr float playerSize = 64.0f;
constexpr float playerSpeed = 250.0f; // pixels per second
constexpr float playerStartX = 60.0f;
constexpr float playerStartY = 200.0f;

constexpr float collectibleSize = 32.0f;
constexpr float collectibleX = 680.0f;
constexpr float collectibleY = 80.0f;
constexpr engine::Color collectibleColor{255, 196, 0, 255};

constexpr float hazardSize = 40.0f;
constexpr float hazardY = 220.0f;
constexpr float hazardMinX = 300.0f;
constexpr float hazardMaxX = 600.0f;
constexpr float hazardSpeed = 180.0f; // pixels per second
constexpr engine::Color hazardColor{200, 60, 60, 255};

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 01 - Continuous"});

    const std::string assetDir = PROTOTYPE_CONTINUOUS_ASSET_DIR;
    const engine::TextureHandle playerTexture = app.LoadTexture((assetDir + "/player.png").c_str());

    float playerX = playerStartX;
    float playerY = playerStartY;

    bool collected = false;
    int score = 0;

    float hazardX = hazardMinX;
    float hazardVelocity = hazardSpeed;

    int hitCount = 0;
    bool wasHitting = false;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        // --- continuous, frame-rate-independent player movement ---
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (app.IsKeyDown(engine::Key::Left) || app.IsKeyDown(engine::Key::A)) {
            moveX -= 1.0f;
        }
        if (app.IsKeyDown(engine::Key::Right) || app.IsKeyDown(engine::Key::D)) {
            moveX += 1.0f;
        }
        if (app.IsKeyDown(engine::Key::Up) || app.IsKeyDown(engine::Key::W)) {
            moveY -= 1.0f;
        }
        if (app.IsKeyDown(engine::Key::Down) || app.IsKeyDown(engine::Key::S)) {
            moveY += 1.0f;
        }

        playerX += moveX * playerSpeed * dt;
        playerY += moveY * playerSpeed * dt;
        playerX = std::clamp(playerX, 0.0f, static_cast<float>(windowWidth) - playerSize);
        playerY = std::clamp(playerY, static_cast<float>(hudHeight), static_cast<float>(windowHeight) - playerSize);

        // --- deterministic hazard motion: bounce between two x positions ---
        hazardX += hazardVelocity * dt;
        if (hazardX > hazardMaxX) {
            hazardX = hazardMaxX;
            hazardVelocity = -hazardSpeed;
        } else if (hazardX < hazardMinX) {
            hazardX = hazardMinX;
            hazardVelocity = hazardSpeed;
        }

        // --- overlap checks (local to the prototype) ---
        const Rect playerBounds{playerX, playerY, playerSize, playerSize};

        if (!collected) {
            const Rect collectibleBounds{collectibleX, collectibleY, collectibleSize, collectibleSize};
            if (Intersects(playerBounds, collectibleBounds)) {
                collected = true;
                ++score;
            }
        }

        const Rect hazardBounds{hazardX, hazardY, hazardSize, hazardSize};
        const bool isHitting = Intersects(playerBounds, hazardBounds);
        if (isHitting && !wasHitting) {
            ++hitCount;
            playerX = playerStartX;
            playerY = playerStartY;
        }
        wasHitting = isHitting;

        app.BeginFrame();
        app.Clear(engine::colors::White);

        if (!collected) {
            app.DrawRectangle(static_cast<int>(collectibleX), static_cast<int>(collectibleY),
                               static_cast<int>(collectibleSize), static_cast<int>(collectibleSize),
                               collectibleColor);
        }

        app.DrawRectangle(static_cast<int>(hazardX), static_cast<int>(hazardY),
                           static_cast<int>(hazardSize), static_cast<int>(hazardSize), hazardColor);

        app.DrawSprite(playerTexture, static_cast<int>(playerX), static_cast<int>(playerY));

        app.DrawText("WASD/Arrows to move. Grab the gold square, avoid the red one.",
                     10, 4, 18, engine::colors::DarkGray);
        app.DrawText("Score: " + std::to_string(score) + "   Hits: " + std::to_string(hitCount),
                     10, 24, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
