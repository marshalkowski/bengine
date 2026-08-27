#include "engine/Engine.hpp"

#include <algorithm>
#include <string>
#include <vector>

// Everything in this file is game-specific: player/projectile/target data,
// firing rules, overlap response, and score are prototype rules, not
// engine capabilities. The engine only supplies window lifecycle, input,
// timing, resources, and rendering (including primitives).

namespace {

constexpr int windowWidth = 800;
constexpr int windowHeight = 450;
constexpr int hudHeight = 44;

constexpr float playerSize = 64.0f;
constexpr float playerSpeed = 220.0f; // pixels per second
constexpr float playerStartX = (windowWidth - playerSize) / 2.0f;
constexpr float playerStartY = hudHeight + (windowHeight - hudHeight - playerSize) / 2.0f;

constexpr float projectileSize = 10.0f;
constexpr float projectileSpeed = 420.0f; // pixels per second
constexpr engine::Color projectileColor{230, 120, 40, 255};

constexpr float targetSize = 36.0f;
constexpr engine::Color targetColor{70, 110, 180, 255};

struct Projectile {
    float x, y;
    float vx, vy;
    bool active = true;
};

struct Target {
    float x, y;
    bool alive = true;
};

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 02 - Projectiles"});

    const std::string assetDir = PROTOTYPE_PROJECTILES_ASSET_DIR;
    const engine::TextureHandle playerTexture = app.LoadTexture((assetDir + "/player.png").c_str());

    float playerX = playerStartX;
    float playerY = playerStartY;

    std::vector<Projectile> projectiles;

    std::vector<Target> targets = {
        {80.0f, 60.0f, true},
        {680.0f, 60.0f, true},
        {80.0f, 370.0f, true},
        {680.0f, 370.0f, true},
        {380.0f, 300.0f, true},
    };

    int score = 0;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        // --- continuous player movement: WASD only; arrows are reserved for firing ---
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (app.IsKeyDown(engine::Key::A)) {
            moveX -= 1.0f;
        }
        if (app.IsKeyDown(engine::Key::D)) {
            moveX += 1.0f;
        }
        if (app.IsKeyDown(engine::Key::W)) {
            moveY -= 1.0f;
        }
        if (app.IsKeyDown(engine::Key::S)) {
            moveY += 1.0f;
        }

        playerX += moveX * playerSpeed * dt;
        playerY += moveY * playerSpeed * dt;
        playerX = std::clamp(playerX, 0.0f, static_cast<float>(windowWidth) - playerSize);
        playerY = std::clamp(playerY, static_cast<float>(hudHeight), static_cast<float>(windowHeight) - playerSize);

        // --- firing: arrow keys are edge-triggered, one press = one projectile ---
        const float originX = playerX + playerSize / 2.0f - projectileSize / 2.0f;
        const float originY = playerY + playerSize / 2.0f - projectileSize / 2.0f;

        if (app.IsKeyPressed(engine::Key::Up)) {
            projectiles.push_back({originX, originY, 0.0f, -projectileSpeed, true});
        }
        if (app.IsKeyPressed(engine::Key::Down)) {
            projectiles.push_back({originX, originY, 0.0f, projectileSpeed, true});
        }
        if (app.IsKeyPressed(engine::Key::Left)) {
            projectiles.push_back({originX, originY, -projectileSpeed, 0.0f, true});
        }
        if (app.IsKeyPressed(engine::Key::Right)) {
            projectiles.push_back({originX, originY, projectileSpeed, 0.0f, true});
        }

        // --- move projectiles, then deactivate on leaving the play area or hitting a target ---
        for (Projectile& projectile : projectiles) {
            projectile.x += projectile.vx * dt;
            projectile.y += projectile.vy * dt;

            const bool outOfBounds = projectile.x + projectileSize < 0.0f || projectile.x > windowWidth ||
                                      projectile.y + projectileSize < hudHeight || projectile.y > windowHeight;
            if (outOfBounds) {
                projectile.active = false;
                continue;
            }

            const engine::Rect projectileBounds{projectile.x, projectile.y, projectileSize, projectileSize};
            for (Target& target : targets) {
                if (!target.alive) {
                    continue;
                }
                const engine::Rect targetBounds{target.x, target.y, targetSize, targetSize};
                if (engine::Intersects(projectileBounds, targetBounds)) {
                    target.alive = false;
                    projectile.active = false;
                    ++score;
                    break;
                }
            }
        }

        std::erase_if(projectiles, [](const Projectile& p) { return !p.active; });

        app.BeginFrame();
        app.Clear(engine::colors::White);

        for (const Target& target : targets) {
            if (target.alive) {
                app.DrawRectangle(static_cast<int>(target.x), static_cast<int>(target.y),
                                   static_cast<int>(targetSize), static_cast<int>(targetSize), targetColor);
            }
        }

        for (const Projectile& projectile : projectiles) {
            app.DrawRectangle(static_cast<int>(projectile.x), static_cast<int>(projectile.y),
                               static_cast<int>(projectileSize), static_cast<int>(projectileSize), projectileColor);
        }

        app.DrawSprite(playerTexture, static_cast<int>(playerX), static_cast<int>(playerY));

        const int targetsRemaining = static_cast<int>(
            std::count_if(targets.begin(), targets.end(), [](const Target& t) { return t.alive; }));

        app.DrawText("WASD to move, Arrow keys to fire", 10, 4, 18, engine::colors::DarkGray);
        app.DrawText(targetsRemaining == 0
                         ? "All targets destroyed!  Score: " + std::to_string(score)
                         : "Targets remaining: " + std::to_string(targetsRemaining) +
                               "   Score: " + std::to_string(score),
                     10, 24, 18, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
