#pragma once

namespace engine {

struct WindowConfig {
    int width = 800;
    int height = 450;
    const char* title = "Engine";
    int targetFPS = 60;
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
};

} // namespace engine
