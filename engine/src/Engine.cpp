#include "engine/Engine.hpp"

#include <cstdio>
#include <unordered_map>
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

bool Intersects(const Rect& a, const Rect& b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}

// The engine's texture resource manager: owns every loaded texture for the
// lifetime of the Engine and deduplicates repeated LoadTexture calls for the
// same path. Deliberately just a vector + a path->index cache for now — no
// per-texture unload, since nothing yet needs it.
struct Engine::Impl {
    std::vector<::Texture2D> textures;
    std::unordered_map<std::string, std::size_t> textureIndexByPath;

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

bool Engine::IsKeyPressed(Key key) const {
    return ::IsKeyPressed(ToRaylibKey(key));
}

bool Engine::IsKeyReleased(Key key) const {
    return ::IsKeyReleased(ToRaylibKey(key));
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

void Engine::DrawRectangle(float x, float y, float width, float height, Color color) {
    ::DrawRectangleRec(::Rectangle{x, y, width, height}, ToRaylibColor(color));
}

void Engine::DrawLine(float x1, float y1, float x2, float y2, Color color) {
    ::DrawLineV(::Vector2{x1, y1}, ::Vector2{x2, y2}, ToRaylibColor(color));
}

TextureHandle Engine::LoadTexture(const char* filePath) {
    const auto existing = impl_->textureIndexByPath.find(filePath);
    if (existing != impl_->textureIndexByPath.end()) {
        return TextureHandle(existing->second);
    }

    impl_->textures.push_back(::LoadTexture(filePath));
    const std::size_t index = impl_->textures.size() - 1;
    impl_->textureIndexByPath.emplace(filePath, index);
    return TextureHandle(index);
}

int Engine::TextureWidth(TextureHandle texture) const {
    return impl_->textures[texture.index_].width;
}

int Engine::TextureHeight(TextureHandle texture) const {
    return impl_->textures[texture.index_].height;
}

void Engine::DrawSprite(TextureHandle texture, float x, float y) {
    ::DrawTextureV(impl_->textures[texture.index_], ::Vector2{x, y}, ::Color{255, 255, 255, 255});
}

void Engine::DrawSpriteRegion(TextureHandle texture, Rect sourceRect, float x, float y) {
    const ::Rectangle source{sourceRect.x, sourceRect.y, sourceRect.width, sourceRect.height};
    ::DrawTextureRec(impl_->textures[texture.index_], source, ::Vector2{x, y}, ::Color{255, 255, 255, 255});
}

int Engine::LoadedTextureCount() const {
    return static_cast<int>(impl_->textures.size());
}

} // namespace engine
