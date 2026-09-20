# E1 — Engine Primitives Proposal

Status: **proposal only, not yet implemented.** Written to support Dethnor's
migration from Godot to Bengine, ahead of its first playable-room milestone.
Dethnor has completed its M0 migration milestone (builds against Bengine,
opens a window, runs the app/scene loop, loads and renders a real asset).

This proposal covers three generic engine primitives the migration has
exposed as missing. Scope is deliberately small — see "Explicitly out of
scope" at the end.

## Method

Before proposing anything, the current Bengine source was inspected:
`engine/include/engine/Engine.hpp`, `engine/src/Engine.cpp`, all 14 examples,
all 6 prototypes, the CMake setup, and the prior prototype architecture
reviews in `docs/`. None of the prior reviews mention camera or asset-path
friction, so this is new territory, not something already considered and
deferred.

**Conventions observed and preserved:** `Engine` is a single class owning
window/frame/resource state behind a `unique_ptr<Impl>`; public types are
small POD structs (`Rect`, `Color`) or opaque index-handle classes
(`TextureHandle`, `SoundHandle`) with private constructors befriended to
`Engine`; free functions live outside `Engine` for pure, state-free
operations (`ToString`, `Intersects`, `Contains`); raylib types never cross
the header boundary; enums are hand-mapped via a `switch` in the `.cpp`,
extended only as concretely needed.

---

## 1. Expanded keyboard input

**Today:** `enum class Key { W, A, S, D, Up, Down, Left, Right, Space }`,
mapped 1:1 to raylib key codes in a private `switch` in `Engine.cpp`.
`IsKeyDown/Pressed/Released` already give correct held/edge semantics.

**Limitation:** Dethnor needs X, Z, C, Enter today, and — being a real game
rather than a single-capability example — will keep needing whatever
ordinary key it binds next. Extending the enum one key at a time per game is
exactly the repetitive-engine-edit friction this pass should remove.

**Proposed API:** Extend the existing `Key` enum (no new type) with the full
set of keys an ordinary keyboard-driven 2D game plausibly binds:

```cpp
enum class Key {
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Digits (top row)
    Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
    // Arrows
    Up, Down, Left, Right,
    // Common controls
    Space, Enter, Escape, Tab, Backspace,
    Shift, Ctrl, Alt,
};
```

**Implementation:** Same pattern — extend the `ToRaylibKey` switch
(`KEY_A`..`KEY_Z`, `KEY_ZERO`..`KEY_NINE`, `KEY_ENTER`, `KEY_ESCAPE`,
`KEY_TAB`, `KEY_BACKSPACE`, `KEY_LEFT_SHIFT`, `KEY_LEFT_CONTROL`,
`KEY_LEFT_ALT`). No changes to `IsKeyDown/Pressed/Released` or the
abstraction boundary.

**Why engine, not game:** Reading a physical key is the textbook
foundational capability — a game cannot do it without reaching past the
engine into raylib. This isn't Dethnor-specific; every keyboard-driven
prototype hits the same wall eventually.

**Deliberately excluded:** function keys, numpad, left/right-distinguished
modifiers, media/system keys, gamepad. None are demonstrated needs yet;
adding them later is a pure enum+switch extension, not a design change.

**Compatibility:** Purely additive — existing values keep their names and
behavior.

---

## 2. Camera2D primitive

**Today:** No camera concept. Every `Draw*` call takes coordinates directly
interpreted as screen pixels (see `09_float_rendering`). There's no
world/screen distinction anywhere in the engine.

**Limitation:** Dethnor needs a scrolling world larger than the window, with
drawing positions expressed in world space and translated to screen space
for a horizontally-panning view. Doing this in game code would mean either
faking it by manually offsetting every draw call's x/y (fragile,
error-prone, and duplicates what raylib's matrix-based camera already solves
correctly), or reaching directly into raylib — which the architecture
forbids.

**Proposed API:**

```cpp
struct Camera2D {
    float x = 0.0f;    // world point centered in the viewport
    float y = 0.0f;
    float zoom = 1.0f; // 1.0 = no scaling
};

struct Vec2 { float x, y; };

class Engine {
    // Between these calls, all existing Draw* calls interpret their
    // coordinates as world space instead of screen space. Nest inside
    // BeginFrame/EndFrame; do not nest camera mode itself.
    void BeginCameraMode(const Camera2D& camera);
    void EndCameraMode();

    // Pure coordinate conversion, usable any time (e.g. converting a mouse
    // click to world space) independent of whether camera mode is active.
    Vec2 WorldToScreen(const Camera2D& camera, float worldX, float worldY) const;
    Vec2 ScreenToWorld(const Camera2D& camera, float screenX, float screenY) const;
};
```

No new draw methods — `DrawSprite`, `DrawSpriteRegion`, `DrawRectangle`,
`DrawLine`, `DrawText` are reused unchanged. Anything drawn between
`BeginCameraMode`/`EndCameraMode` is world-space (scrolls with the camera);
anything drawn outside it (HUD, menus) stays screen-space, exactly like
raylib's own `BeginMode2D`/`EndMode2D` convention.

**Implementation:** Internally builds a
`::Camera2D{ offset = {ScreenWidth/2, ScreenHeight/2}, target = {camera.x, camera.y}, rotation = 0, zoom = camera.zoom }`
and calls raylib's `BeginMode2D`/`EndMode2D`. `WorldToScreen`/`ScreenToWorld`
build the same conversion and call raylib's `GetWorldToScreen2D`/
`GetScreenToWorld2D`. Screen size is read live via raylib's
`GetScreenWidth/Height` rather than cached, so it stays correct if the
window is ever resizable later.

**Why engine, not game:** this requires wrapping raylib's projection-matrix
machinery (`BeginMode2D`/`GetWorldToScreen2D`/etc.) — a game can't implement
it without crossing the raylib boundary directly. It's a foundational
rendering-adjacent primitive, and camera math is identical regardless of
genre.

**Deliberately excluded (stays in game code):** following behavior,
smooth-damp/lerp, dead-zones, bounds/zone clamping, scripted movement,
rotation (raylib's `Camera2D` supports it, but nothing demonstrates a need
yet — trivial to add later since the internal conversion already has a
`rotation` field to wire up). Dethnor implements horizontal-follow itself by
writing to `Camera2D.x`/`.y` each frame using the primitive above.

**Note:** introduces one small new POD type, `Vec2`, parallel to the
existing `Rect` — needed because coordinate transforms are inherently
paired x/y, unlike `MouseX()/MouseY()`'s independent scalars. Flagging this
explicitly since it's the one piece slightly beyond the minimum literal ask.

**Compatibility:** Fully additive; nothing existing changes unless a
prototype opts into `BeginCameraMode`.

---

## 3. Asset path/root handling

**Today:** Every example/prototype CMakeLists.txt bakes an absolute
source-tree path into a per-target compile definition
(`FOO_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"`), and `main.cpp` does
`std::string(FOO_ASSET_DIR) + "/sprite.png"` at each call site (see
`01_hello_sprite/CMakeLists.txt:15`, repeated 12 times).
`01_hello_sprite/CMakeLists.txt` even comments that this is deliberate
because "the engine has no asset-loading/search-path concept yet."

**Limitation:** This works for one flat `assets/` folder per tiny example,
but two things break down for Dethnor: (1) it's a compile-time,
source-tree-relative path — fine for local dev, but meaningless for a
distributed/installed build where assets ship next to the .exe, not next to
source; and (2) it doesn't address the VS-debugger-vs-direct-launch
working-directory split at all (it sidesteps CWD entirely by hardcoding an
absolute path, which stops working the moment the game isn't run from its
own build tree).

**Proposed API:** Lives on `Engine`, since it must integrate with the
existing texture/sound caches rather than create a parallel system — a
standalone `AssetManager` would duplicate `LoadTexture`/`LoadSound`'s
caching.

```cpp
// Resolves relative paths at runtime, robust to the executable's current
// working directory (VS debugger vs. launching the .exe directly).
std::string ExecutableDirectory(); // free function, raylib GetApplicationDirectory() internally

class Engine {
    // Establishes the base directory subsequent relative LoadTexture/
    // LoadSound/ResolveAssetPath calls resolve against. Optional -- an
    // unset root leaves all three behaving exactly as today (paths used
    // as given). Absolute paths passed to any of them always bypass the
    // root, so existing examples' baked absolute paths keep working
    // unmodified.
    void SetAssetRoot(const std::string& root);

    // Same resolution LoadTexture/LoadSound use internally, exposed for
    // a game's own non-texture/sound assets (e.g. level data it loads itself).
    std::string ResolveAssetPath(const std::string& relativePath) const;
};
```

Typical Dethnor usage:
`app.SetAssetRoot(engine::ExecutableDirectory() + "/assets"); app.LoadTexture("knight/idle.png");`

**Implementation:** One private `ResolveAssetPath`-equivalent helper in
`Impl`, using `std::filesystem::path(p).is_absolute()` to decide whether to
prepend the root. `LoadTexture`/`LoadSound` route their `filePath` through
it before doing the existing cache lookup/`::LoadTexture`/`::LoadSound`
call — cache keying is unaffected since it keys on the resolved string.
`ExecutableDirectory()` wraps raylib's `GetApplicationDirectory()`, which is
correct regardless of CWD on both launch paths — better than parsing
`argv[0]`, which isn't reliable.

**Why engine, not game:** resolving relative-to-an-executable paths
correctly across platforms means either wrapping a raylib/OS call
(`GetApplicationDirectory`) or hand-rolling platform-specific logic —
either way it's boundary-crossing territory, and the resolution logic must
live next to the caches it affects to avoid a second bookkeeping system.

**Deliberately excluded:** no multiple search paths/mod support, no
per-resource-type subroots, no directory scanning, no hot-reload. A single
flat root is the smallest primitive that solves the demonstrated problem;
multiple roots can be added later if evidence shows up.

**Compatibility:** Fully additive and backward-compatible — every existing
example already passes fully-baked absolute paths, which remain untouched
since `SetAssetRoot` is opt-in and absolute paths always bypass the root.

---

## Summary of new surface

- `Key`: ~35 new enumerators, same enum, same mapping pattern.
- New types: `Camera2D`, `Vec2`. New `Engine` methods: `BeginCameraMode`,
  `EndCameraMode`, `WorldToScreen`, `ScreenToWorld`.
- New `Engine` methods: `SetAssetRoot`, `ResolveAssetPath`. New free
  function: `ExecutableDirectory`.
- No existing public signature changes; nothing in prior `docs/` prototype
  reviews conflicts with any of this.

## Explicitly out of scope for E1

ECS, scene management, JSON parsing, a UI framework, AI systems,
Dethnor-specific character/camera/zone logic, a general tweening system, a
large asset manager. These may be considered separately later.

## Open questions for review

- `Vec2` naming/shape, or an alternative return convention for
  `WorldToScreen`/`ScreenToWorld`.
- Which modifier/control keys actually belong in the first pass vs. later.
- Whether `SetAssetRoot` should instead be a `WindowConfig` field set at
  construction rather than a separate method.
