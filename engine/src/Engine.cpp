#include "engine/Engine.hpp"

#include <cstdio>
#include <utility>
#include <vector>

#include "raylib.h"

namespace {

::Color ToRaylibColor(engine::Color color) {
    return {color.r, color.g, color.b, color.a};
}

int ToRaylibKey(engine::Key key) {
    switch (key) {
        case engine::Key::W: return KEY_W;
        case engine::Key::A: return KEY_A;
        case engine::Key::S: return KEY_S;
        case engine::Key::D: return KEY_D;
        case engine::Key::Up: return KEY_UP;
        case engine::Key::Down: return KEY_DOWN;
        case engine::Key::Left: return KEY_LEFT;
        case engine::Key::Right: return KEY_RIGHT;
    }
    return KEY_NULL;
}

} // namespace

namespace engine {

std::string ToString(float value, int decimalPlaces) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", decimalPlaces, value);
    return std::string(buffer);
}

// The engine's texture resource manager: owns every loaded texture for the
// lifetime of the Engine. Deliberately just a flat vector for now — no
// caching/dedup and no per-texture unload, since nothing yet needs either.
struct Engine::Impl {
    std::vector<::Texture2D> textures;

    ~Impl() {
        for (::Texture2D& texture : textures) {
            UnloadTexture(texture);
        }
    }
};

Engine::Engine(const WindowConfig& config) : impl_(std::make_unique<Impl>()) {
    InitWindow(config.width, config.height, config.title);
    SetTargetFPS(config.targetFPS);
}

Engine::~Engine() {
    // Textures must be unloaded while the GL context is still alive, so
    // destroy the resource manager before tearing down the window.
    impl_.reset();
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

float Engine::DeltaTime() const {
    return GetFrameTime();
}

bool Engine::IsKeyDown(Key key) const {
    return ::IsKeyDown(ToRaylibKey(key));
}

void Engine::Clear(Color color) {
    ClearBackground(ToRaylibColor(color));
}

void Engine::DrawText(const char* text, int x, int y, int fontSize, Color color) {
    ::DrawText(text, x, y, fontSize, ToRaylibColor(color));
}

void Engine::DrawText(const std::string& text, int x, int y, int fontSize, Color color) {
    DrawText(text.c_str(), x, y, fontSize, color);
}

TextureHandle Engine::LoadTexture(const char* filePath) {
    impl_->textures.push_back(::LoadTexture(filePath));
    return TextureHandle(impl_->textures.size() - 1);
}

int Engine::TextureWidth(TextureHandle texture) const {
    return impl_->textures[texture.index_].width;
}

int Engine::TextureHeight(TextureHandle texture) const {
    return impl_->textures[texture.index_].height;
}

void Engine::DrawSprite(TextureHandle texture, int x, int y) {
    ::DrawTexture(impl_->textures[texture.index_], x, y, ::Color{255, 255, 255, 255});
}

} // namespace engine
