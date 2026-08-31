#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace engine {

// Formats a float with a fixed number of decimal places, e.g. ToString(0.5f, 2) -> "0.50".
std::string ToString(float value, int decimalPlaces = 2);

struct WindowConfig {
    int width = 800;
    int height = 450;
    const char* title = "Engine";
    int targetFPS = 60;
};

struct Color {
    unsigned char r, g, b, a;
};

namespace colors {
inline constexpr Color White{245, 245, 245, 255};
inline constexpr Color DarkGray{80, 80, 80, 255};
} // namespace colors

// Axis-aligned rectangle, in whatever coordinate space the caller is
// working in. Pure data — no rendering or backend dependency.
struct Rect {
    float x, y, width, height;
};

// True if a and b share a region of positive area. Rectangles that only
// touch along an edge or at a corner (zero-width or zero-height overlap)
// are NOT considered intersecting.
bool Intersects(const Rect& a, const Rect& b);

// True if the zero-area point (x, y) lies within rect, treating rect's
// right/bottom edges as exclusive. Deliberately not expressed in terms of
// Intersects: a point has no area, so Intersects' positive-area-overlap
// rule would call every point non-intersecting, including points well
// inside the rectangle. This is the basis for pointer hit-testing.
bool Contains(const Rect& rect, float x, float y);

// Static definition of a sprite-sheet animation: a run of equal-size,
// equal-duration frames laid out horizontally in one row, starting at
// firstFrame. Pure data — reusable across any number of Animation
// instances playing it, and independent of any specific texture.
struct AnimationClip {
    int firstFrame;
    int frameCount;
    float frameWidth;
    float frameHeight;
    float frameDuration;
    bool loop;
};

// Playback state for an AnimationClip. Advances via explicit Update() calls
// using caller-supplied delta time — never reads time itself. Only
// produces the current frame's source Rect; drawing and texture ownership
// remain the application's responsibility.
class Animation {
public:
    explicit Animation(const AnimationClip& clip);

    void Update(float deltaTime);

    // Returns to frame zero and clears completion, whether or not the
    // clip had finished.
    void Restart();

    // Always false for a looping clip. For a one-shot clip, true once
    // playback has reached and held on the final frame.
    bool IsComplete() const;

    Rect CurrentFrameRect() const;

private:
    AnimationClip clip_;
    int currentFrame_;
    float accumulatedTime_;
    bool completed_;
};

// Physical keyboard keys. Only the keys concrete examples have needed so far;
// extend as new examples require more.
enum class Key {
    W,
    A,
    S,
    D,
    Up,
    Down,
    Left,
    Right,
    Space,
};

// Physical mouse buttons. Only the button a point-and-click prototype has
// needed so far; extend as new prototypes require more (see Key above).
enum class MouseButton {
    Left,
};

// Opaque reference to a texture resource owned by the engine. Carries no
// data or methods of its own — application code copies/stores/passes it,
// but only Engine can create one or make sense of what it refers to.
// Textures are loaded once and live for the lifetime of the Engine that
// loaded them (see Engine::LoadTexture).
class TextureHandle {
private:
    friend class Engine;

    explicit TextureHandle(std::size_t index) : index_(index) {}

    std::size_t index_;
};

// Opaque reference to a short sound effect owned by the engine. Same
// shape and lifetime as TextureHandle: application code copies/stores/
// passes it, only Engine can create one or make sense of it, and it
// lives for the lifetime of the Engine that loaded it (see
// Engine::LoadSound). Deliberately a separate type from TextureHandle
// rather than a shared template — two resource kinds isn't enough
// evidence to generalize the pattern, and the two are loaded from
// separate caches (see Engine::LoadSound).
class SoundHandle {
private:
    friend class Engine;

    explicit SoundHandle(std::size_t index) : index_(index) {}

    std::size_t index_;
};

// Owns window + frame lifecycle. Does not own main() or the game loop itself —
// the application calls ShouldClose()/BeginFrame()/EndFrame() from its own loop.
// Also owns the audio device: initialized alongside the window in the
// constructor and torn down in the destructor, so no example/game needs
// to remember to set audio up itself, same as nothing sets up the window
// manually today.
class Engine {
public:
    explicit Engine(const WindowConfig& config);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool ShouldClose() const;
    void BeginFrame();
    void EndFrame();

    // Seconds elapsed since the previous frame.
    float DeltaTime() const;

    // True every frame the key is physically held down.
    bool IsKeyDown(Key key) const;

    // True only during the frame the key transitions from up to down.
    bool IsKeyPressed(Key key) const;

    // True only during the frame the key transitions from down to up.
    bool IsKeyReleased(Key key) const;

    // Cursor position in window pixel coordinates, the same space every
    // other x/y in this API already uses (DrawRectangle, entity positions,
    // etc.) — so a caller can hit-test it against gameplay/UI rects with
    // Contains() directly, no coordinate conversion.
    float MouseX() const;
    float MouseY() const;

    // True only during the frame the button transitions from up to down —
    // the click edge. No IsMouseButtonDown/Released yet: nothing has
    // needed held-button or release detection so far (see MouseButton).
    bool IsMouseButtonPressed(MouseButton button) const;

    void Clear(Color color);
    void DrawText(const char* text, int x, int y, int fontSize, Color color);
    void DrawText(const std::string& text, int x, int y, int fontSize, Color color);
    void DrawRectangle(float x, float y, float width, float height, Color color);
    void DrawLine(float x1, float y1, float x2, float y2, Color color);

    // Loads a texture and hands back a handle to it. The engine owns the
    // texture from this point on; there is no explicit unload — all loaded
    // textures are released when this Engine is destroyed. Repeated calls
    // with the same filePath reuse the already-loaded texture instead of
    // loading it again.
    TextureHandle LoadTexture(const char* filePath);
    int TextureWidth(TextureHandle texture) const;
    int TextureHeight(TextureHandle texture) const;
    void DrawSprite(TextureHandle texture, float x, float y);

    // Draws only sourceRect (in texture pixel coordinates) from texture at
    // the given destination position — the basis for sprite-sheet frames.
    void DrawSpriteRegion(TextureHandle texture, Rect sourceRect, float x, float y);

    // Diagnostic: how many distinct textures are currently loaded. Useful
    // for verifying that repeated LoadTexture calls are being deduplicated;
    // not meant to be a basis for game logic.
    int LoadedTextureCount() const;

    // Loads a short sound effect and hands back a handle to it, caching by
    // filePath exactly like LoadTexture — repeated calls with the same
    // path reuse the already-loaded sound. There is no explicit unload;
    // all loaded sounds are released when this Engine is destroyed.
    SoundHandle LoadSound(const char* filePath);

    // Triggers playback once and returns immediately — not tied to
    // BeginFrame/EndFrame like the Draw* methods, since it has no visual
    // output. Can be called anywhere in the loop. No stop/volume/looping/
    // instance control: this is deliberately just "play it."
    void PlaySound(SoundHandle sound);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace engine
