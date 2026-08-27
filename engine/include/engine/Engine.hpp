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

// Owns window + frame lifecycle. Does not own main() or the game loop itself —
// the application calls ShouldClose()/BeginFrame()/EndFrame() from its own loop.
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

    void Clear(Color color);
    void DrawText(const char* text, int x, int y, int fontSize, Color color);
    void DrawText(const std::string& text, int x, int y, int fontSize, Color color);
    void DrawRectangle(int x, int y, int width, int height, Color color);
    void DrawLine(int x1, int y1, int x2, int y2, Color color);

    // Loads a texture and hands back a handle to it. The engine owns the
    // texture from this point on; there is no explicit unload — all loaded
    // textures are released when this Engine is destroyed. Repeated calls
    // with the same filePath reuse the already-loaded texture instead of
    // loading it again.
    TextureHandle LoadTexture(const char* filePath);
    int TextureWidth(TextureHandle texture) const;
    int TextureHeight(TextureHandle texture) const;
    void DrawSprite(TextureHandle texture, int x, int y);

    // Diagnostic: how many distinct textures are currently loaded. Useful
    // for verifying that repeated LoadTexture calls are being deduplicated;
    // not meant to be a basis for game logic.
    int LoadedTextureCount() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace engine
