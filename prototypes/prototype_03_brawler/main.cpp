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
// notion of "idle"/"walk"/"attack"/"hurt"/"defeat", nor any notion that a
// "knight" or "skeleton" exists. Each clip here also uses its own
// dedicated texture file (the real asset pack is one-file-per-action),
// so the prototype selects the active texture alongside the active
// animation — Animation itself never knows which texture goes with it.

namespace {

constexpr int windowWidth = 800;
constexpr int windowHeight = 450;
constexpr int hudHeight = 44;

// Real sprite frames are 128x64 (wider than tall, to fit outstretched
// attack poses). Gameplay collision bounds are deliberately NOT the same
// as the visual frame size — see playerSize/enemySize below.
constexpr float spriteFrameWidth = 128.0f;
constexpr float spriteFrameHeight = 64.0f;

// Gameplay collision box: an explicit, prototype-owned approximation of
// each character's body, independent of the sprite artwork's full extent
// (which includes padding for outstretched slash frames).
constexpr float playerSize = 64.0f;
constexpr float playerSpeed = 200.0f; // pixels per second

constexpr float enemySize = 64.0f;
constexpr float enemySpeed = 40.0f; // pixels per second
constexpr int enemyMaxHealth = 3;

constexpr float attackWidth = 40.0f;
constexpr float attackVisualDuration = 0.15f;
constexpr float attackImpactHoldDuration = 0.12f; // extra hold on the final (impact) attack frame
constexpr engine::Color attackColor{230, 200, 60, 255};

constexpr float exitSize = 50.0f;
constexpr float exitX = 720.0f;
constexpr float exitY = 180.0f;
constexpr engine::Color exitLockedColor{150, 150, 150, 255};
constexpr engine::Color exitOpenColor{80, 180, 90, 255};

// Clip definitions below are transcribed from ANIMATION_METADATA.md, the
// human-filled-in source of truth for this asset pack. Each clip's
// firstFrame is 0 since every action is its own single-row sheet (no
// sharing a sheet across clips, unlike the earlier placeholder assets).
constexpr engine::AnimationClip knightIdleClip{
    .firstFrame = 0, .frameCount = 2, .frameWidth = spriteFrameWidth, .frameHeight = spriteFrameHeight,
    .frameDuration = 0.4f, .loop = true};
constexpr engine::AnimationClip knightWalkClip{
    .firstFrame = 0, .frameCount = 6, .frameWidth = spriteFrameWidth, .frameHeight = spriteFrameHeight,
    .frameDuration = 0.167f, .loop = true};
constexpr engine::AnimationClip knightAttackClip{
    .firstFrame = 0, .frameCount = 5, .frameWidth = spriteFrameWidth, .frameHeight = spriteFrameHeight,
    .frameDuration = 0.167f, .loop = false};

constexpr engine::AnimationClip skeletonIdleClip{
    .firstFrame = 0, .frameCount = 2, .frameWidth = spriteFrameWidth, .frameHeight = spriteFrameHeight,
    .frameDuration = 0.5f, .loop = true};
constexpr engine::AnimationClip skeletonHurtClip{
    .firstFrame = 0, .frameCount = 2, .frameWidth = spriteFrameWidth, .frameHeight = spriteFrameHeight,
    .frameDuration = 0.5f, .loop = false};
constexpr engine::AnimationClip skeletonDefeatClip{
    .firstFrame = 0, .frameCount = 9, .frameWidth = spriteFrameWidth, .frameHeight = spriteFrameHeight,
    .frameDuration = 0.167f, .loop = false};

enum class Facing { Left, Right };

struct Player {
    float x, y;
    Facing facing = Facing::Right;
    bool isAttacking = false;
    float attackHoldTimer = 0.0f;
    engine::Animation idleAnimation{knightIdleClip};
    engine::Animation walkAnimation{knightWalkClip};
    engine::Animation attackAnimation{knightAttackClip};
};

struct Enemy {
    float x, y;
    int health = enemyMaxHealth;
    bool alive = true;
    bool isHurt = false;
    Facing facing = Facing::Right;
    engine::Animation idleAnimation{skeletonIdleClip};
    engine::Animation hurtAnimation{skeletonHurtClip};
    engine::Animation defeatAnimation{skeletonDefeatClip};

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

// Sprite frames are wider than the gameplay collision box; center the
// visual frame horizontally over the box rather than aligning their
// left edges (visual bounds are not gameplay bounds).
float SpriteDrawX(float entityX, float entitySize) {
    return entityX - (spriteFrameWidth - entitySize) / 2.0f;
}

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 03 - Brawler"});

    const std::string assetDir = PROTOTYPE_BRAWLER_ASSET_DIR;
    const engine::TextureHandle knightIdleTexture =
        app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Idle-2.png").c_str());
    const engine::TextureHandle knightWalkTexture =
        app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Walk.png").c_str());
    const engine::TextureHandle knightAttackTexture =
        app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Strike-Fwd.png").c_str());

    const engine::TextureHandle skeletonIdleTexture =
        app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Idle-2.png").c_str());
    const engine::TextureHandle skeletonHurtTexture =
        app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Hit.png").c_str());
    const engine::TextureHandle skeletonDefeatTexture =
        app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Fall.png").c_str());

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
            player.attackHoldTimer = 0.0f;
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
        // The attack clip's own final frame is the "impact" pose; once it
        // completes, hold on that frame a little longer before returning to
        // walk/idle. Animation already stays on its last frame indefinitely
        // once complete, so this only needs a small prototype-local timer —
        // no engine change.
        if (player.isAttacking) {
            if (!player.attackAnimation.IsComplete()) {
                player.attackAnimation.Update(dt);
            } else {
                player.attackHoldTimer += dt;
                if (player.attackHoldTimer >= attackImpactHoldDuration) {
                    player.isAttacking = false;
                }
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

            // Face toward the player, same left/right convention as the player.
            if (dx < 0.0f) {
                enemy.facing = Facing::Left;
            } else if (dx > 0.0f) {
                enemy.facing = Facing::Right;
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
                const engine::TextureHandle enemyTexture = enemy.isHurt ? skeletonHurtTexture : skeletonIdleTexture;
                engine::Rect frame =
                    enemy.isHurt ? enemy.hurtAnimation.CurrentFrameRect() : enemy.idleAnimation.CurrentFrameRect();
                if (enemy.facing == Facing::Left) {
                    frame.width = -frame.width;
                }
                app.DrawSpriteRegion(enemyTexture, frame, SpriteDrawX(enemy.x, enemySize), enemy.y);
            } else if (!enemy.defeatAnimation.IsComplete()) {
                engine::Rect frame = enemy.defeatAnimation.CurrentFrameRect();
                if (enemy.facing == Facing::Left) {
                    frame.width = -frame.width;
                }
                app.DrawSpriteRegion(skeletonDefeatTexture, frame, SpriteDrawX(enemy.x, enemySize), enemy.y);
            }
        }

        if (attackVisualTimer > 0.0f) {
            const engine::Rect attackBounds = AttackBounds(player);
            app.DrawRectangle(attackBounds.x, attackBounds.y, attackBounds.width, attackBounds.height, attackColor);
        }

        const engine::TextureHandle playerTexture = player.isAttacking  ? knightAttackTexture
                                                     : isMoving          ? knightWalkTexture
                                                                         : knightIdleTexture;
        engine::Rect playerFrame = player.isAttacking  ? player.attackAnimation.CurrentFrameRect()
                                    : isMoving          ? player.walkAnimation.CurrentFrameRect()
                                                        : player.idleAnimation.CurrentFrameRect();
        if (player.facing == Facing::Left) {
            // Negative source width flips the sprite horizontally — raylib's
            // DrawTextureRec/DrawTexturePro already support this, so no
            // engine change was needed to add facing.
            playerFrame.width = -playerFrame.width;
        }
        app.DrawSpriteRegion(playerTexture, playerFrame, SpriteDrawX(player.x, playerSize), player.y);

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
