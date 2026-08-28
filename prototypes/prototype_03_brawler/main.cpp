#include "engine/Engine.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

// Everything in this file is game-specific: player/enemy data, facing,
// attack rules, enemy movement, encounter/exit state, and now scene
// flow (Title/Brawler/Win) are all rules of this prototype, not engine
// capabilities. The engine only supplies window lifecycle, input, timing,
// resources, geometry, and rendering (including primitives, sprite
// regions, and clip playback via engine::Animation). The engine has no
// notion of "scene" at all — see docs/prototype_03_brawler_scene_flow_review.md
// for what that exercise revealed.
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
    // Set each frame by UpdateBrawler from that frame's input, and read
    // back by DrawBrawler to pick the matching texture/animation. Before
    // this exercise split update and draw into separate functions, this
    // was just a local in the single per-frame loop body; splitting update
    // and draw meant a decision made during update (which clip should be
    // playing) needed an explicit home to survive until draw. See the
    // architecture review's "Update/draw routing" section.
    bool isMoving = false;
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

// ---------------------------------------------------------------------
// Scene flow. This prototype is deliberately explicit and hand-rolled:
// the engine has no Scene/SceneManager concept as of this exercise (see
// the architecture review). Title, Brawler, and Win are just an enum tag
// plus per-state data and a pair of switches in main() routing update and
// draw to whichever one is active. Nothing below is engine API.
// ---------------------------------------------------------------------

enum class AppState { Title, Brawler, Win };

// Textures are application-lifetime resources: loaded once, reused across
// every gameplay session, and never reloaded on restart (the engine has
// no unload/reload primitive, and reloading identical files each restart
// would be wasted work anyway). This is a different lifetime than the
// gameplay data below, which is intentionally destroyed and rebuilt per
// session — see the "Ownership" section of the architecture review.
struct BrawlerAssets {
    engine::TextureHandle knightIdle;
    engine::TextureHandle knightWalk;
    engine::TextureHandle knightAttack;
    engine::TextureHandle skeletonIdle;
    engine::TextureHandle skeletonHurt;
    engine::TextureHandle skeletonDefeat;
};

BrawlerAssets LoadBrawlerAssets(engine::Engine& app) {
    const std::string assetDir = PROTOTYPE_BRAWLER_ASSET_DIR;
    return BrawlerAssets{
        .knightIdle = app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Idle-2.png").c_str()),
        .knightWalk = app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Walk.png").c_str()),
        .knightAttack = app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Strike-Fwd.png").c_str()),
        .skeletonIdle = app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Idle-2.png").c_str()),
        .skeletonHurt = app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Hit.png").c_str()),
        .skeletonDefeat = app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Fall.png").c_str()),
    };
}

// Everything a single brawler encounter needs: player, enemies, and the
// small bits of encounter-local timing/completion state. Grouping these
// in one struct (rather than loose locals, as before this exercise) is
// what makes "destroy the whole session" and "construct a whole fresh
// session" into single, obvious operations instead of a checklist of
// individual fields to remember to reset.
struct BrawlerState {
    Player player{80.0f, 200.0f};
    std::vector<Enemy> enemies;
    float attackVisualTimer = 0.0f;
    bool encounterComplete = false;
};

// A fresh, ready-to-play encounter, built from scratch every time gameplay
// is (re)entered. Nothing from a previous run — player position, enemy
// health, attack timers, animation playback position — can leak into a
// new session, because nothing is reused; this constructs an entirely new
// BrawlerState value rather than mutating an old one back to defaults.
BrawlerState MakeFreshBrawlerState() {
    BrawlerState state;
    state.enemies.emplace_back(500.0f, 120.0f);
    state.enemies.emplace_back(620.0f, 320.0f);
    return state;
}

// All per-frame gameplay simulation for one brawler session. Only called
// while AppState::Brawler is active — see the routing switch in main().
void UpdateBrawler(BrawlerState& state, engine::Engine& app, float dt) {
    Player& player = state.player;
    std::vector<Enemy>& enemies = state.enemies;

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

    player.isMoving = moveX != 0.0f || moveY != 0.0f;

    // --- attack: one dedicated key, one damage evaluation per press ---
    // Damage still lands synchronously on the key press, independent of
    // which attack frame happens to be showing (see architecture review
    // for why this is an acceptable simplification for now).
    if (state.attackVisualTimer > 0.0f) {
        state.attackVisualTimer -= dt;
    }

    if (app.IsKeyPressed(engine::Key::Up)) {
        player.isAttacking = true;
        player.attackHoldTimer = 0.0f;
        player.attackAnimation.Restart();
        state.attackVisualTimer = attackVisualDuration;

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
    } else if (player.isMoving) {
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

    if (allEnemiesDefeated && !state.encounterComplete) {
        const engine::Rect exitBounds{exitX, exitY, exitSize, exitSize};
        if (engine::Intersects(PlayerBounds(player), exitBounds)) {
            state.encounterComplete = true;
        }
    }
}

// All rendering for one brawler session. Only called while AppState::Brawler
// is active — see the routing switch in main(). Purely reads state/assets;
// never mutates gameplay data.
void DrawBrawler(engine::Engine& app, const BrawlerState& state, const BrawlerAssets& assets) {
    const Player& player = state.player;
    const std::vector<Enemy>& enemies = state.enemies;
    const bool allEnemiesDefeated =
        std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.alive; });

    app.DrawRectangle(exitX, exitY, exitSize, exitSize, allEnemiesDefeated ? exitOpenColor : exitLockedColor);

    for (const Enemy& enemy : enemies) {
        if (enemy.alive) {
            const engine::TextureHandle enemyTexture = enemy.isHurt ? assets.skeletonHurt : assets.skeletonIdle;
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
            app.DrawSpriteRegion(assets.skeletonDefeat, frame, SpriteDrawX(enemy.x, enemySize), enemy.y);
        }
    }

    if (state.attackVisualTimer > 0.0f) {
        const engine::Rect attackBounds = AttackBounds(player);
        app.DrawRectangle(attackBounds.x, attackBounds.y, attackBounds.width, attackBounds.height, attackColor);
    }

    // player.isMoving is set each frame by UpdateBrawler and read back
    // here — see the comment on Player::isMoving for why that field exists.
    const engine::TextureHandle playerTexture = player.isAttacking  ? assets.knightAttack
                                                 : player.isMoving   ? assets.knightWalk
                                                                     : assets.knightIdle;
    engine::Rect playerFrame = player.isAttacking  ? player.attackAnimation.CurrentFrameRect()
                                : player.isMoving   ? player.walkAnimation.CurrentFrameRect()
                                                     : player.idleAnimation.CurrentFrameRect();
    if (player.facing == Facing::Left) {
        // Negative source width flips the sprite horizontally — raylib's
        // DrawTextureRec/DrawTexturePro already support this, so no
        // engine change was needed to add facing.
        playerFrame.width = -playerFrame.width;
    }
    app.DrawSpriteRegion(playerTexture, playerFrame, SpriteDrawX(player.x, playerSize), player.y);

    app.DrawText("WASD to move, Up arrow to attack", 10, 4, 18, engine::colors::DarkGray);

    if (state.encounterComplete) {
        app.DrawText("Encounter complete!", 10, 24, 18, engine::colors::DarkGray);
    } else if (allEnemiesDefeated) {
        app.DrawText("All enemies defeated - reach the exit!", 10, 24, 18, engine::colors::DarkGray);
    } else {
        const int remaining = static_cast<int>(
            std::count_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.alive; }));
        app.DrawText("Enemies remaining: " + std::to_string(remaining), 10, 24, 18, engine::colors::DarkGray);
    }
}

void DrawTitle(engine::Engine& app) {
    app.DrawText("PROTOTYPE 03: BRAWLER", 220, 180, 32, engine::colors::DarkGray);
    app.DrawText("Press Up to start", 300, 240, 20, engine::colors::DarkGray);
}

void DrawWin(engine::Engine& app) {
    app.DrawText("Encounter Complete!", 250, 180, 32, engine::colors::DarkGray);
    app.DrawText("Press Up to play again", 260, 240, 20, engine::colors::DarkGray);
}

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 03 - Brawler"});
    const BrawlerAssets assets = LoadBrawlerAssets(app);

    AppState state = AppState::Title;

    // Engaged (has_value()) only while state == AppState::Brawler. Entering
    // gameplay constructs a fresh BrawlerState via emplace(); leaving it
    // resets() the optional, actually destroying the player/enemy/timer
    // data rather than merely hiding it. There is deliberately no path
    // that mutates an existing BrawlerState back to its initial values —
    // "start fresh" is always "construct a new one", never "reset fields".
    std::optional<BrawlerState> brawler;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        // Update routing: only the active state's simulation runs. Title
        // and Win have no simulation to run at all (no enemies drifting,
        // no animations advancing) — "don't run gameplay behind the title
        // screen" falls out for free from simply not calling UpdateBrawler
        // unless state == Brawler.
        switch (state) {
        case AppState::Title:
            if (app.IsKeyPressed(engine::Key::Up)) {
                brawler.emplace(MakeFreshBrawlerState());
                state = AppState::Brawler;
            }
            break;
        case AppState::Brawler:
            UpdateBrawler(*brawler, app, dt);
            if (brawler->encounterComplete) {
                brawler.reset(); // discard the finished session before entering Win
                state = AppState::Win;
            }
            break;
        case AppState::Win:
            if (app.IsKeyPressed(engine::Key::Up)) {
                brawler.emplace(MakeFreshBrawlerState());
                state = AppState::Brawler;
            }
            break;
        }

        // Draw routing mirrors update routing exactly: one active state,
        // one draw call. Scenes are mutually exclusive for this exercise —
        // no overlay/stack, so this is a plain switch rather than a list.
        app.BeginFrame();
        app.Clear(engine::colors::White);
        switch (state) {
        case AppState::Title:
            DrawTitle(app);
            break;
        case AppState::Brawler:
            DrawBrawler(app, *brawler, assets);
            break;
        case AppState::Win:
            DrawWin(app);
            break;
        }
        app.EndFrame();
    }

    return 0;
}
