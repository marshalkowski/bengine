#include "engine/Engine.hpp"

#include "raylib.h"

namespace {

::Color ToRaylibColor(engine::Color color) {
    return {color.r, color.g, color.b, color.a};
}

} // namespace

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

void Engine::Clear(Color color) {
    ClearBackground(ToRaylibColor(color));
}

void Engine::DrawText(const char* text, int x, int y, int fontSize, Color color) {
    ::DrawText(text, x, y, fontSize, ToRaylibColor(color));
}

} // namespace engine
