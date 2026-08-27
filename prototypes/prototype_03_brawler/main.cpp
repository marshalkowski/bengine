#include "engine/Engine.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// Everything in this file is game-specific: player/enemy data, facing,
// attack rules, enemy movement, and encounter/exit state are rules of this
// prototype, not engine capabilities. The engine only supplies window
// lifecycle, input, timing, resources, geometry, and rendering (including
// primitives, sprite regions, and clip playback via engine::Animation).
//
// Which animation is active, when one-shots restart, and when playback
// returns to a looping clip are all decided here — Animation itself has no
// notion of "idle"/"walk"/"attack"/"hurt"/"defeat".

namespace {

constexpr int windowWidth = 800;
constexpr int windowHeight = 450;
constexpr int hudHeight = 44;

constexpr float playerSize = 64.0f;
constexpr float playerSpeed = 200.0f; // pixels per second

constexpr float enemySize = 64.0f;
constexpr float enemySpeed = 40.0f; // pixels per second
constexpr int enemyMaxHealth = 3;

constexpr float attackWidth = 40.0f;
constexpr float attackVisualDuration = 0.15f;
constexpr engine::Color attackColor{230, 200, 60, 255};

constexpr float exitSize = 50.0f;
constexpr float exitX = 720.0f;
constexpr float exitY = 180.0f;
constexpr engine::Color exitLockedColor{150, 150, 150, 255};
constexpr engine::Color exitOpenColor{80, 180, 90, 255};

// Player sprite sheet: idle(2) + walk(4) + attack(3) = 9 frames, one row.
constexpr engine::AnimationClip playerIdleClip{
    .firstFrame = 0, .frameCount = 2, .frameWidth = 64.0f, .frameHeight = 64.0f, .frameDuration = 0.3f, .loop = true};
constexpr engine::AnimationClip playerWalkClip{
    .firstFrame = 2, .frameCount = 4, .frameWidth = 64.0f, .frameHeight = 64.0f, .frameDuration = 0.12f, .loop = true};
constexpr engine::AnimationClip playerAttackClip{
    .firstFrame = 6, .frameCount = 3, .frameWidth = 64.0f, .frameHeight = 64.0f, .frameDuration = 0.08f, .loop = false};

// Enemy sprite sheet: idle(3) + hurt(2) + defeat(3) = 8 frames, one row.
constexpr engine::AnimationClip enemyIdleClip{
    .firstFrame = 0, .frameCount = 3, .frameWidth = 64.0f, .frameHeight = 64.0f, .frameDuration = 0.2f, .loop = true};
constexpr engine::AnimationClip enemyHurtClip{
    .firstFrame = 3, .frameCount = 2, .frameWidth = 64.0f, .frameHeight = 64.0f, .frameDuration = 0.1f, .loop = false};
constexpr engine::AnimationClip enemyDefeatClip{
    .firstFrame = 5, .frameCount = 3, .frameWidth = 64.0f, .frameHeight = 64.0f, .frameDuration = 0.15f, .loop = false};

enum class Facing { Left, Right };

struct Player {
    float x, y;
    Facing facing = Facing::Right;
    bool isAttacking = false;
    engine::Animation idleAnimation{playerIdleClip};
    engine::Animation walkAnimation{playerWalkClip};
    engine::Animation attackAnimation{playerAttackClip};
};

struct Enemy {
    float x, y;
    int health = enemyMaxHealth;
    bool alive = true;
    bool isHurt = false;
    engine::Animation idleAnimation{enemyIdleClip};
    engine::Animation hurtAnimation{enemyHurtClip};
    engine::Animation defeatAnimation{enemyDefeatClip};

    Enemy(float startX, float startY) : x(startX), y(startY) {}
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
    const engine::TextureHandle playerTexture = app.LoadTexture((assetDir + "/player_sheet.png").c_str());
    const engine::TextureHandle enemyTexture = app.LoadTexture((assetDir + "/enemy_sheet.png").c_str());

    Player player{80.0f, 200.0f};

    std::vector<Enemy> enemies;
    enemies.emplace_back(500.0f, 120.0f);
    enemies.emplace_back(620.0f, 320.0f);

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

        const bool isMoving = moveX != 0.0f || moveY != 0.0f;

        // --- attack: one dedicated key, one damage evaluation per press ---
        // Damage still lands synchronously on the key press, independent of
        // which attack frame happens to be showing (see architecture review
        // for why this is an acceptable simplification for now).
        if (attackVisualTimer > 0.0f) {
            attackVisualTimer -= dt;
        }

        if (app.IsKeyPressed(engine::Key::Up)) {
            player.isAttacking = true;
            player.attackAnimation.Restart();
            attackVisualTimer = attackVisualDuration;

            const engine::Rect attackBounds = AttackBounds(player);
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) {
                    continue;
                }
                if (engine::Intersects(attackBounds, EnemyBounds(enemy))) {
                    --enemy.health;
                    enemy.isHurt = true;
                    enemy.hurtAnimation.Restart();
                    if (enemy.health <= 0) {
                        enemy.alive = false;
                        enemy.isHurt = false;
                        enemy.defeatAnimation.Restart();
                    }
                }
            }
        }

        // --- player animation selection: attacking beats moving beats idle ---
        if (player.isAttacking) {
            player.attackAnimation.Update(dt);
            if (player.attackAnimation.IsComplete()) {
                player.isAttacking = false;
            }
        } else if (isMoving) {
            player.walkAnimation.Update(dt);
        } else {
            player.idleAnimation.Update(dt);
        }

        // --- enemies drift slowly toward the player and animate accordingly ---
        for (Enemy& enemy : enemies) {
            if (!enemy.alive) {
                if (!enemy.defeatAnimation.IsComplete()) {
                    enemy.defeatAnimation.Update(dt);
                }
                continue;
            }

            const float dx = player.x - enemy.x;
            const float dy = player.y - enemy.y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            if (distance > 1.0f) {
                enemy.x += (dx / distance) * enemySpeed * dt;
                enemy.y += (dy / distance) * enemySpeed * dt;
            }

            if (enemy.isHurt) {
                enemy.hurtAnimation.Update(dt);
                if (enemy.hurtAnimation.IsComplete()) {
                    enemy.isHurt = false;
                }
            } else {
                enemy.idleAnimation.Update(dt);
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
            if (enemy.alive) {
                const engine::Rect frame =
                    enemy.isHurt ? enemy.hurtAnimation.CurrentFrameRect() : enemy.idleAnimation.CurrentFrameRect();
                app.DrawSpriteRegion(enemyTexture, frame, enemy.x, enemy.y);
            } else if (!enemy.defeatAnimation.IsComplete()) {
                app.DrawSpriteRegion(enemyTexture, enemy.defeatAnimation.CurrentFrameRect(), enemy.x, enemy.y);
            }
        }

        if (attackVisualTimer > 0.0f) {
            const engine::Rect attackBounds = AttackBounds(player);
            app.DrawRectangle(attackBounds.x, attackBounds.y, attackBounds.width, attackBounds.height, attackColor);
        }

        engine::Rect playerFrame = player.isAttacking  ? player.attackAnimation.CurrentFrameRect()
                                    : isMoving          ? player.walkAnimation.CurrentFrameRect()
                                                        : player.idleAnimation.CurrentFrameRect();
        if (player.facing == Facing::Left) {
            // Negative source width flips the sprite horizontally — raylib's
            // DrawTextureRec/DrawTexturePro already support this, so no
            // engine change was needed to add facing.
            playerFrame.width = -playerFrame.width;
        }
        app.DrawSpriteRegion(playerTexture, playerFrame, player.x, player.y);

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
