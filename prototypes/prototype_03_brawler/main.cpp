#include "engine/Engine.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// Everything in this file is game-specific: player/enemy data, facing,
// attack rules, enemy movement, and encounter/exit state are rules of this
// prototype, not engine capabilities. The engine only supplies window
// lifecycle, input, timing, resources, geometry, and rendering (including
// primitives).

namespace {

constexpr int windowWidth = 800;
constexpr int windowHeight = 450;
constexpr int hudHeight = 44;

constexpr float playerSize = 64.0f;
constexpr float playerSpeed = 200.0f; // pixels per second

constexpr float enemySize = 48.0f;
constexpr float enemySpeed = 40.0f; // pixels per second
constexpr int enemyMaxHealth = 3;
constexpr engine::Color enemyColor{140, 70, 150, 255};
constexpr engine::Color enemyHurtColor{230, 230, 230, 255};

constexpr float attackWidth = 40.0f;
constexpr float attackVisualDuration = 0.15f;
constexpr float hurtFlashDuration = 0.15f;
constexpr engine::Color attackColor{230, 200, 60, 255};

constexpr float exitSize = 50.0f;
constexpr float exitX = 720.0f;
constexpr float exitY = 180.0f;
constexpr engine::Color exitLockedColor{150, 150, 150, 255};
constexpr engine::Color exitOpenColor{80, 180, 90, 255};

enum class Facing { Left, Right };

struct Player {
    float x, y;
    Facing facing = Facing::Right;
};

struct Enemy {
    float x, y;
    int health = enemyMaxHealth;
    bool alive = true;
    float hurtFlashTimer = 0.0f;
};

engine::Rect PlayerBounds(const Player& player) {
    return engine::Rect{player.x, player.y, playerSize, playerSize};
}

engine::Rect EnemyBounds(const Enemy& enemy) {
    return engine::Rect{enemy.x, enemy.y, enemySize, enemySize};
}

// The attack region sits immediately beside the player, on whichever side
// they're currently facing.
engine::Rect AttackBounds(const Player& player) {
    if (player.facing == Facing::Right) {
        return engine::Rect{player.x + playerSize, player.y, attackWidth, playerSize};
    }
    return engine::Rect{player.x - attackWidth, player.y, attackWidth, playerSize};
}

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 03 - Brawler"});

    const std::string assetDir = PROTOTYPE_BRAWLER_ASSET_DIR;
    const engine::TextureHandle playerTexture = app.LoadTexture((assetDir + "/player.png").c_str());

    Player player{80.0f, 200.0f, Facing::Right};

    std::vector<Enemy> enemies = {
        Enemy{500.0f, 120.0f},
        Enemy{620.0f, 320.0f},
    };

    float attackVisualTimer = 0.0f;
    bool encounterComplete = false;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        // --- player movement: WASD only; an arrow key is reserved for attacking ---
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

        if (moveX < 0.0f) {
            player.facing = Facing::Left;
        } else if (moveX > 0.0f) {
            player.facing = Facing::Right;
        }

        player.x += moveX * playerSpeed * dt;
        player.y += moveY * playerSpeed * dt;
        player.x = std::clamp(player.x, 0.0f, static_cast<float>(windowWidth) - playerSize);
        player.y = std::clamp(player.y, static_cast<float>(hudHeight), static_cast<float>(windowHeight) - playerSize);

        // --- enemies drift slowly toward the player ---
        for (Enemy& enemy : enemies) {
            if (!enemy.alive) {
                continue;
            }

            const float dx = player.x - enemy.x;
            const float dy = player.y - enemy.y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            if (distance > 1.0f) {
                enemy.x += (dx / distance) * enemySpeed * dt;
                enemy.y += (dy / distance) * enemySpeed * dt;
            }

            if (enemy.hurtFlashTimer > 0.0f) {
                enemy.hurtFlashTimer -= dt;
            }
        }

        // --- attack: one dedicated key, one damage evaluation per press ---
        if (attackVisualTimer > 0.0f) {
            attackVisualTimer -= dt;
        }

        if (app.IsKeyPressed(engine::Key::Up)) {
            attackVisualTimer = attackVisualDuration;
            const engine::Rect attackBounds = AttackBounds(player);
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) {
                    continue;
                }
                if (engine::Intersects(attackBounds, EnemyBounds(enemy))) {
                    --enemy.health;
                    enemy.hurtFlashTimer = hurtFlashDuration;
                    if (enemy.health <= 0) {
                        enemy.alive = false;
                    }
                }
            }
        }

        // --- encounter completion: all enemies defeated, then reach the exit ---
        const bool allEnemiesDefeated =
            std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.alive; });

        if (allEnemiesDefeated && !encounterComplete) {
            const engine::Rect exitBounds{exitX, exitY, exitSize, exitSize};
            if (engine::Intersects(PlayerBounds(player), exitBounds)) {
                encounterComplete = true;
            }
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawRectangle(exitX, exitY, exitSize, exitSize, allEnemiesDefeated ? exitOpenColor : exitLockedColor);

        for (const Enemy& enemy : enemies) {
            if (!enemy.alive) {
                continue;
            }
            const engine::Color color = enemy.hurtFlashTimer > 0.0f ? enemyHurtColor : enemyColor;
            app.DrawRectangle(enemy.x, enemy.y, enemySize, enemySize, color);
        }

        if (attackVisualTimer > 0.0f) {
            const engine::Rect attackBounds = AttackBounds(player);
            app.DrawRectangle(attackBounds.x, attackBounds.y, attackBounds.width, attackBounds.height, attackColor);
        }

        app.DrawSprite(playerTexture, player.x, player.y);

        app.DrawText("WASD to move, Up arrow to attack", 10, 4, 18, engine::colors::DarkGray);

        if (encounterComplete) {
            app.DrawText("Encounter complete!", 10, 24, 18, engine::colors::DarkGray);
        } else if (allEnemiesDefeated) {
            app.DrawText("All enemies defeated - reach the exit!", 10, 24, 18, engine::colors::DarkGray);
        } else {
            const int remaining = static_cast<int>(
                std::count_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.alive; }));
            app.DrawText("Enemies remaining: " + std::to_string(remaining), 10, 24, 18, engine::colors::DarkGray);
        }

        app.EndFrame();
    }

    return 0;
}
