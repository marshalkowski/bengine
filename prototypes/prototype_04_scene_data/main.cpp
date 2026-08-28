#include "engine/Engine.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

// This prototype pressure-tests scene flow a second time, now with two
// deliberately *different* gameplay scenes (Combat, reusing brawler-style
// melee, and Hazard, an avoidance scene with no combat at all) and, for
// the first time, scene content loaded from external text files instead
// of being baked into constants. Everything below -- scene
// representation, transitions, the file format, and how external data
// becomes runtime state -- is prototype/game code. The engine supplies
// only window lifecycle, input, timing, resources, geometry, and
// rendering, exactly as before; nothing here is new engine API. See
// docs/prototype_04_scene_data_review.md for what this exercise revealed.

namespace {

constexpr int windowWidth = 800;
constexpr int windowHeight = 450;

constexpr float spriteFrameWidth = 128.0f;
constexpr float spriteFrameHeight = 64.0f;

enum class Facing { Left, Right };

// Sprite frames are wider than any gameplay collision box; center the
// visual frame horizontally over the box rather than aligning left edges.
float SpriteDrawX(float entityX, float entitySize) {
    return entityX - (spriteFrameWidth - entitySize) / 2.0f;
}

// ---------------------------------------------------------------------
// Shared character assets. Textures are application-lifetime resources
// (loaded once, reused by every scene and every run, never reloaded) --
// same choice the previous prototype made, unchanged here. Both Combat
// and Hazard read from this one struct; Hazard just never uses the
// attack texture.
// ---------------------------------------------------------------------

struct CharacterAssets {
    engine::TextureHandle knightIdle;
    engine::TextureHandle knightWalk;
    engine::TextureHandle knightAttack;
    engine::TextureHandle skeletonIdle;
    engine::TextureHandle skeletonHurt;
    engine::TextureHandle skeletonDefeat;
};

CharacterAssets LoadCharacterAssets(engine::Engine& app) {
    const std::string assetDir = PROTOTYPE_SCENE_DATA_ASSET_DIR;
    return CharacterAssets{
        .knightIdle = app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Idle-2.png").c_str()),
        .knightWalk = app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Walk.png").c_str()),
        .knightAttack = app.LoadTexture((assetDir + "/knight/MBEU_character_knight-Strike-Fwd.png").c_str()),
        .skeletonIdle = app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Idle-2.png").c_str()),
        .skeletonHurt = app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Hit.png").c_str()),
        .skeletonDefeat = app.LoadTexture((assetDir + "/skeleton/MBEU_character_skeleton-Fall.png").c_str()),
    };
}

// ---------------------------------------------------------------------
// Tiny external-data reader. This is the entire "file format" for this
// prototype: blank lines and '#' comments are dropped, every remaining
// line is split into whitespace-separated tokens. No nesting, no generic
// key/value object model, no schema shared between Combat and Hazard --
// each scene's loader below recognizes its own small, fixed set of
// directive names and interprets its own tokens. This is deliberately
// not a serialization framework; it is exactly enough structure for two
// short, flat directive lists.
//
// engine::Engine has no file-reading API, and none was added for this --
// reading a small text file is std::ifstream, not a rendering, audio, or
// platform concern the raylib-backed engine layer would need to own.
// ---------------------------------------------------------------------

std::vector<std::vector<std::string>> ReadDirectiveLines(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::vector<std::string>> lines;
    std::string rawLine;
    while (std::getline(file, rawLine)) {
        std::istringstream lineStream(rawLine);
        std::vector<std::string> tokens;
        std::string token;
        while (lineStream >> token) {
            tokens.push_back(token);
        }
        if (tokens.empty() || tokens[0][0] == '#') {
            continue;
        }
        lines.push_back(std::move(tokens));
    }
    return lines;
}

// =======================================================================
// Combat scene: brawler-style melee, carried over from prototype_03_brawler.
// =======================================================================

constexpr float playerSize = 64.0f;
constexpr float playerSpeed = 200.0f;

constexpr float enemySize = 64.0f;
constexpr float enemySpeed = 40.0f;
constexpr int enemyMaxHealth = 3;

constexpr float attackWidth = 40.0f;
constexpr float attackVisualDuration = 0.15f;
constexpr float attackImpactHoldDuration = 0.12f;
constexpr engine::Color attackColor{230, 200, 60, 255};

constexpr engine::Color exitLockedColor{150, 150, 150, 255};
constexpr engine::Color exitOpenColor{80, 180, 90, 255};

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

struct Player {
    float x, y;
    Facing facing = Facing::Right;
    bool isAttacking = false;
    bool isMoving = false; // set by UpdateCombat, read by DrawCombat -- see prototype_03's review for why this exists
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

engine::Rect PlayerBounds(float x, float y) {
    return engine::Rect{x, y, playerSize, playerSize};
}

engine::Rect EnemyBounds(const Enemy& enemy) {
    return engine::Rect{enemy.x, enemy.y, enemySize, enemySize};
}

engine::Rect AttackBounds(const Player& player) {
    if (player.facing == Facing::Right) {
        return engine::Rect{player.x + playerSize, player.y, attackWidth, playerSize};
    }
    return engine::Rect{player.x - attackWidth, player.y, attackWidth, playerSize};
}

// --- External Combat scene content -------------------------------------
//
// What actually varies per Combat encounter: where the player starts,
// where enemies start, the playfield bounds, and where the exit is. What
// stays code (deliberately NOT data): collision sizes, movement/attack
// speed, enemy health, attack timing, and the rule that all enemies must
// die before the exit unlocks -- all gameplay behavior, not placement.

struct EnemySpawn {
    float x, y;
};

struct CombatSceneData {
    engine::Rect bounds{};
    float playerSpawnX = 0.0f;
    float playerSpawnY = 0.0f;
    std::vector<EnemySpawn> enemies;
    engine::Rect exit{};
};

CombatSceneData LoadCombatSceneData(const std::string& path) {
    CombatSceneData data;
    for (const std::vector<std::string>& tokens : ReadDirectiveLines(path)) {
        const std::string& directive = tokens[0];
        if (directive == "bounds") {
            data.bounds = engine::Rect{
                std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4])};
        } else if (directive == "player_spawn") {
            data.playerSpawnX = std::stof(tokens[1]);
            data.playerSpawnY = std::stof(tokens[2]);
        } else if (directive == "enemy") {
            data.enemies.push_back(EnemySpawn{std::stof(tokens[1]), std::stof(tokens[2])});
        } else if (directive == "exit") {
            data.exit = engine::Rect{
                std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4])};
        }
        // Unrecognized directives are silently ignored. Fine for this
        // experiment's scope; a real content pipeline would want to
        // reject or at least warn on typos and out-of-range values --
        // this prototype does no such validation anywhere.
    }
    return data;
}

// --- Combat runtime state -----------------------------------------------
//
// Everything one Combat session needs, grouped exactly the way
// BrawlerState was in the previous prototype: constructing a new value
// from CombatSceneData is "start fresh"; there is no reset-in-place path.

struct CombatState {
    Player player;
    std::vector<Enemy> enemies;
    float attackVisualTimer = 0.0f;
    bool combatComplete = false;
};

CombatState MakeFreshCombatState(const CombatSceneData& data) {
    CombatState state;
    state.player.x = data.playerSpawnX;
    state.player.y = data.playerSpawnY;
    for (const EnemySpawn& spawn : data.enemies) {
        state.enemies.emplace_back(spawn.x, spawn.y);
    }
    return state;
}

void UpdateCombat(CombatState& state, const CombatSceneData& data, engine::Engine& app, float dt) {
    Player& player = state.player;
    std::vector<Enemy>& enemies = state.enemies;

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
    player.x = std::clamp(player.x, data.bounds.x, data.bounds.x + data.bounds.width - playerSize);
    player.y = std::clamp(player.y, data.bounds.y, data.bounds.y + data.bounds.height - playerSize);

    player.isMoving = moveX != 0.0f || moveY != 0.0f;

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

    const bool allEnemiesDefeated =
        std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.alive; });

    if (allEnemiesDefeated && !state.combatComplete) {
        if (engine::Intersects(PlayerBounds(player.x, player.y), data.exit)) {
            state.combatComplete = true;
        }
    }
}

void DrawCombat(engine::Engine& app, const CombatState& state, const CombatSceneData& data,
                const CharacterAssets& assets) {
    const Player& player = state.player;
    const std::vector<Enemy>& enemies = state.enemies;
    const bool allEnemiesDefeated =
        std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.alive; });

    app.DrawRectangle(data.exit.x, data.exit.y, data.exit.width, data.exit.height,
                       allEnemiesDefeated ? exitOpenColor : exitLockedColor);

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

    const engine::TextureHandle playerTexture = player.isAttacking  ? assets.knightAttack
                                                 : player.isMoving   ? assets.knightWalk
                                                                     : assets.knightIdle;
    engine::Rect playerFrame = player.isAttacking  ? player.attackAnimation.CurrentFrameRect()
                                : player.isMoving   ? player.walkAnimation.CurrentFrameRect()
                                                     : player.idleAnimation.CurrentFrameRect();
    if (player.facing == Facing::Left) {
        playerFrame.width = -playerFrame.width;
    }
    app.DrawSpriteRegion(playerTexture, playerFrame, SpriteDrawX(player.x, playerSize), player.y);

    app.DrawText("WASD to move, Up arrow to attack", 10, 4, 18, engine::colors::DarkGray);

    if (state.combatComplete) {
        app.DrawText("Encounter complete!", 10, 24, 18, engine::colors::DarkGray);
    } else if (allEnemiesDefeated) {
        app.DrawText("All enemies defeated - reach the exit!", 10, 24, 18, engine::colors::DarkGray);
    } else {
        const int remaining = static_cast<int>(
            std::count_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.alive; }));
        app.DrawText("Enemies remaining: " + std::to_string(remaining), 10, 24, 18, engine::colors::DarkGray);
    }
}

// =======================================================================
// Hazard scene: avoidance, not combat. No health, no attacking, no
// per-object animation set beyond the player's own idle/walk -- and a
// kind of runtime object (a patrolling hazard) Combat has no equivalent
// of at all: no health, no facing, no animation, just a position that
// oscillates on its own.
// =======================================================================

constexpr float hazardSize = 36.0f;
constexpr engine::Color hazardColor{200, 70, 70, 255};

enum class HazardAxis { Horizontal, Vertical };

// --- External Hazard scene content --------------------------------------
//
// A hazard's placement, patrol axis, range, and speed are data; its size
// and the fact that touching one resets progress are code -- the same
// placement-is-data/behavior-is-code split Combat uses, applied to an
// entirely different kind of object.

struct HazardSpawn {
    HazardAxis axis;
    float anchorX, anchorY;
    float range;
    float speed;
};

struct HazardSceneData {
    engine::Rect bounds{};
    float playerSpawnX = 0.0f;
    float playerSpawnY = 0.0f;
    float surviveSeconds = 5.0f;
    std::vector<HazardSpawn> hazards;
    engine::Rect exit{};
};

HazardSceneData LoadHazardSceneData(const std::string& path) {
    HazardSceneData data;
    for (const std::vector<std::string>& tokens : ReadDirectiveLines(path)) {
        const std::string& directive = tokens[0];
        if (directive == "bounds") {
            data.bounds = engine::Rect{
                std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4])};
        } else if (directive == "player_spawn") {
            data.playerSpawnX = std::stof(tokens[1]);
            data.playerSpawnY = std::stof(tokens[2]);
        } else if (directive == "survive_seconds") {
            data.surviveSeconds = std::stof(tokens[1]);
        } else if (directive == "hazard") {
            const HazardAxis axis = tokens[1] == "vertical" ? HazardAxis::Vertical : HazardAxis::Horizontal;
            data.hazards.push_back(HazardSpawn{
                axis, std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]), std::stof(tokens[5])});
        } else if (directive == "exit") {
            data.exit = engine::Rect{
                std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4])};
        }
    }
    return data;
}

// --- Hazard runtime state ------------------------------------------------

// Deliberately not a reuse of Player: no attack state, no attack
// animation, nothing about combat. Sharing the movement/animation-
// selection logic below with UpdateCombat was considered and rejected for
// this pass -- see the architecture review for why the duplication was
// left visible instead of factored out.
struct HazardAvatar {
    float x, y;
    Facing facing = Facing::Right;
    bool isMoving = false;
    engine::Animation idleAnimation{knightIdleClip};
    engine::Animation walkAnimation{knightWalkClip};
};

// A patrolling hazard: position oscillates along one axis between
// [anchor - range, anchor + range], reversing direction at each end. No
// health, no facing, no animation -- a genuinely different shape of
// runtime object than Enemy, not just an Enemy with different numbers.
struct Hazard {
    HazardAxis axis;
    float anchorX, anchorY;
    float range;
    float speed;
    float offset = 0.0f;
    float direction = 1.0f;
};

engine::Rect HazardBounds(const Hazard& hazard) {
    const float x = hazard.anchorX + (hazard.axis == HazardAxis::Horizontal ? hazard.offset : 0.0f);
    const float y = hazard.anchorY + (hazard.axis == HazardAxis::Vertical ? hazard.offset : 0.0f);
    return engine::Rect{x, y, hazardSize, hazardSize};
}

void UpdateHazardPatrol(Hazard& hazard, float dt) {
    hazard.offset += hazard.direction * hazard.speed * dt;
    if (hazard.offset > hazard.range) {
        hazard.offset = hazard.range;
        hazard.direction = -1.0f;
    } else if (hazard.offset < -hazard.range) {
        hazard.offset = -hazard.range;
        hazard.direction = 1.0f;
    }
}

struct HazardState {
    HazardAvatar player;
    std::vector<Hazard> hazards;
    float surviveTimer = 0.0f;
    bool exitUnlocked = false;
    bool hazardComplete = false;
    bool wasTouchingHazard = false;
    int hazardHits = 0;
};

HazardState MakeFreshHazardState(const HazardSceneData& data) {
    HazardState state;
    state.player.x = data.playerSpawnX;
    state.player.y = data.playerSpawnY;
    for (const HazardSpawn& spawn : data.hazards) {
        Hazard hazard;
        hazard.axis = spawn.axis;
        hazard.anchorX = spawn.anchorX;
        hazard.anchorY = spawn.anchorY;
        hazard.range = spawn.range;
        hazard.speed = spawn.speed;
        state.hazards.push_back(hazard);
    }
    return state;
}

// This function's shape deliberately parallels UpdateCombat's movement
// block almost line-for-line -- see the architecture review's "Combat vs.
// Hazard" section for what that duplication is worth noting.
void UpdateHazard(HazardState& state, const HazardSceneData& data, engine::Engine& app, float dt) {
    HazardAvatar& player = state.player;

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
    player.x = std::clamp(player.x, data.bounds.x, data.bounds.x + data.bounds.width - playerSize);
    player.y = std::clamp(player.y, data.bounds.y, data.bounds.y + data.bounds.height - playerSize);

    player.isMoving = moveX != 0.0f || moveY != 0.0f;

    if (player.isMoving) {
        player.walkAnimation.Update(dt);
    } else {
        player.idleAnimation.Update(dt);
    }

    for (Hazard& hazard : state.hazards) {
        UpdateHazardPatrol(hazard, dt);
    }

    const engine::Rect playerBounds = PlayerBounds(player.x, player.y);
    bool touchingHazard = false;
    for (const Hazard& hazard : state.hazards) {
        if (engine::Intersects(playerBounds, HazardBounds(hazard))) {
            touchingHazard = true;
            break;
        }
    }

    if (touchingHazard && !state.wasTouchingHazard) {
        ++state.hazardHits;
        if (!state.exitUnlocked) {
            state.surviveTimer = 0.0f;
        }
        player.x = data.playerSpawnX;
        player.y = data.playerSpawnY;
    }
    state.wasTouchingHazard = touchingHazard;

    if (!touchingHazard && !state.exitUnlocked) {
        state.surviveTimer += dt;
        if (state.surviveTimer >= data.surviveSeconds) {
            state.exitUnlocked = true;
        }
    }

    if (state.exitUnlocked && !state.hazardComplete) {
        if (engine::Intersects(PlayerBounds(player.x, player.y), data.exit)) {
            state.hazardComplete = true;
        }
    }
}

void DrawHazard(engine::Engine& app, const HazardState& state, const HazardSceneData& data,
                 const CharacterAssets& assets) {
    app.DrawRectangle(data.exit.x, data.exit.y, data.exit.width, data.exit.height,
                       state.exitUnlocked ? exitOpenColor : exitLockedColor);

    for (const Hazard& hazard : state.hazards) {
        const engine::Rect bounds = HazardBounds(hazard);
        app.DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, hazardColor);
    }

    const HazardAvatar& player = state.player;
    const engine::TextureHandle playerTexture = player.isMoving ? assets.knightWalk : assets.knightIdle;
    engine::Rect playerFrame =
        player.isMoving ? player.walkAnimation.CurrentFrameRect() : player.idleAnimation.CurrentFrameRect();
    if (player.facing == Facing::Left) {
        playerFrame.width = -playerFrame.width;
    }
    app.DrawSpriteRegion(playerTexture, playerFrame, SpriteDrawX(player.x, playerSize), player.y);

    app.DrawText("WASD to move - avoid the hazards!", 10, 4, 18, engine::colors::DarkGray);

    if (state.hazardComplete) {
        app.DrawText("Hazard cleared!", 10, 24, 18, engine::colors::DarkGray);
    } else if (state.exitUnlocked) {
        app.DrawText("Survived - reach the exit!", 10, 24, 18, engine::colors::DarkGray);
    } else {
        app.DrawText("Survive: " + engine::ToString(state.surviveTimer, 1) + "/" +
                          engine::ToString(data.surviveSeconds, 1),
                      10, 24, 18, engine::colors::DarkGray);
    }
}

// =======================================================================
// Title / Win, and the run-level (not scene-level) stats that survive
// Combat -> Hazard -> Win. RunStats is application/run-lifetime data:
// unlike CombatState/HazardState it is never destroyed on a scene exit,
// only reset when a fresh run begins (Title -> Combat or Win -> Combat).
// =======================================================================

struct RunStats {
    int enemiesDefeated = 0;
    int hazardHits = 0;
};

void DrawTitle(engine::Engine& app) {
    app.DrawText("PROTOTYPE 04: SCENE DATA", 190, 170, 32, engine::colors::DarkGray);
    app.DrawText("Combat, then Hazard, then Win", 250, 210, 18, engine::colors::DarkGray);
    app.DrawText("Press Up to start", 300, 250, 20, engine::colors::DarkGray);
}

void DrawWin(engine::Engine& app, const RunStats& stats) {
    app.DrawText("Run Complete!", 300, 150, 32, engine::colors::DarkGray);
    app.DrawText("Enemies defeated: " + std::to_string(stats.enemiesDefeated), 280, 210, 20,
                  engine::colors::DarkGray);
    app.DrawText("Hazard hits taken: " + std::to_string(stats.hazardHits), 280, 235, 20, engine::colors::DarkGray);
    app.DrawText("Press Up to play again", 260, 280, 20, engine::colors::DarkGray);
}

// =======================================================================
// Scene flow: one active-state enum, one scene-local session container
// per gameplay scene, two routing switches. Same shape as
// prototype_03_brawler's Title/Brawler/Win, now with two gameplay scenes
// instead of one -- see the architecture review for what that changed.
// =======================================================================

enum class AppState { Title, Combat, Hazard, Win };

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 04 - Scene Data"});
    const CharacterAssets assets = LoadCharacterAssets(app);

    const std::string sceneDir = std::string(PROTOTYPE_SCENE_DATA_ASSET_DIR) + "/scenes";
    const CombatSceneData combatData = LoadCombatSceneData(sceneDir + "/combat_scene.txt");
    const HazardSceneData hazardData = LoadHazardSceneData(sceneDir + "/hazard_scene.txt");

    AppState state = AppState::Title;

    // One optional per gameplay scene, following the pattern that worked
    // in the previous prototype: engaged only while that scene is active,
    // constructed fresh via emplace() on entry, destroyed via reset() on
    // exit. Never both engaged at once -- see the architecture review for
    // whether having two of these instead of one still felt clean.
    std::optional<CombatState> combat;
    std::optional<HazardState> hazard;

    // Run-lifetime, not scene-lifetime: survives every scene transition
    // within a run, reset only when a fresh run begins.
    RunStats runStats;

    while (!app.ShouldClose()) {
        const float dt = app.DeltaTime();

        switch (state) {
        case AppState::Title:
            if (app.IsKeyPressed(engine::Key::Up)) {
                runStats = RunStats{};
                combat.emplace(MakeFreshCombatState(combatData));
                state = AppState::Combat;
            }
            break;
        case AppState::Combat:
            UpdateCombat(*combat, combatData, app, dt);
            if (combat->combatComplete) {
                // Combat -> Hazard: a transition between two substantial
                // gameplay scenes, not through Title/Win. Nothing about
                // Combat's player position, enemy state, or timers
                // carries forward -- Hazard starts entirely from its own
                // scene data. Only the derived summary value (how many
                // enemies were defeated) crosses over, into run-lifetime
                // state, not into Hazard's own scene state.
                runStats.enemiesDefeated = static_cast<int>(combat->enemies.size());
                combat.reset();
                hazard.emplace(MakeFreshHazardState(hazardData));
                state = AppState::Hazard;
            }
            break;
        case AppState::Hazard:
            UpdateHazard(*hazard, hazardData, app, dt);
            if (hazard->hazardComplete) {
                runStats.hazardHits = hazard->hazardHits;
                hazard.reset();
                state = AppState::Win;
            }
            break;
        case AppState::Win:
            if (app.IsKeyPressed(engine::Key::Up)) {
                runStats = RunStats{};
                combat.emplace(MakeFreshCombatState(combatData));
                state = AppState::Combat;
            }
            break;
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);
        switch (state) {
        case AppState::Title:
            DrawTitle(app);
            break;
        case AppState::Combat:
            DrawCombat(app, *combat, combatData, assets);
            break;
        case AppState::Hazard:
            DrawHazard(app, *hazard, hazardData, assets);
            break;
        case AppState::Win:
            DrawWin(app, runStats);
            break;
        }
        app.EndFrame();
    }

    return 0;
}
