#include "engine/Engine.hpp"

#include <utility>

#include "raylib.h"

namespace {

::Color ToRaylibColor(engine::Color color) {
    return {color.r, color.g, color.b, color.a};
}

} // namespace

namespace engine {

struct Texture::Impl {
    ::Texture2D raylibTexture;

    explicit Impl(::Texture2D texture) : raylibTexture(texture) {}
    ~Impl() { UnloadTexture(raylibTexture); }
};

Texture::Texture(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
Texture::~Texture() = default;
Texture::Texture(Texture&&) noexcept = default;
Texture& Texture::operator=(Texture&&) noexcept = default;

int Texture::Width() const {
    return impl_->raylibTexture.width;
}

int Texture::Height() const {
    return impl_->raylibTexture.height;
}

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

Texture Engine::LoadTexture(const char* filePath) {
    return Texture(std::make_unique<Texture::Impl>(::LoadTexture(filePath)));
}

void Engine::DrawSprite(const Texture& texture, int x, int y) {
    ::DrawTexture(texture.impl_->raylibTexture, x, y, ::Color{255, 255, 255, 255});
}

} // namespace engine
