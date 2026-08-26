#include "engine/Engine.hpp"

#include "raylib.h"

namespace engine {

Engine::Engine(const WindowConfig& config) {
    InitWindow(config.width, config.height, config.title);
    SetTargetFPS(config.targetFPS);
}

Engine::~Engine() {
    CloseWindow();
}

bool Engine::ShouldClose() const {
    return WindowShouldClose();
}

void Engine::BeginFrame() {
    BeginDrawing();
}

void Engine::EndFrame() {
    EndDrawing();
}

} // namespace engine
