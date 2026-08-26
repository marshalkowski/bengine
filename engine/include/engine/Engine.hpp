#pragma once

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

// RAII handle for a texture loaded on the GPU. Only Engine can create one
// (via LoadTexture); the backing resource is released when it goes out of scope.
class Texture {
public:
    ~Texture();

    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    int Width() const;
    int Height() const;

private:
    friend class Engine;

    struct Impl;
    explicit Texture(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;
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

    void Clear(Color color);
    void DrawText(const char* text, int x, int y, int fontSize, Color color);
    void DrawText(const std::string& text, int x, int y, int fontSize, Color color);

    Texture LoadTexture(const char* filePath);
    void DrawSprite(const Texture& texture, int x, int y);
};

} // namespace engine
