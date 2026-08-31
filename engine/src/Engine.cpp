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
        case engine::Key::Space: return KEY_SPACE;
    }
    return KEY_NULL;
}

int ToRaylibMouseButton(engine::MouseButton button) {
    switch (button) {
        case engine::MouseButton::Left: return MOUSE_BUTTON_LEFT;
    }
    return MOUSE_BUTTON_LEFT;
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

bool Contains(const Rect& rect, float x, float y) {
    return x >= rect.x && x < rect.x + rect.width &&
           y >= rect.y && y < rect.y + rect.height;
}

Animation::Animation(const AnimationClip& clip)
    : clip_(clip), currentFrame_(0), accumulatedTime_(0.0f), completed_(false) {}

void Animation::Update(float deltaTime) {
    if (completed_) {
        return;
    }

    accumulatedTime_ += deltaTime;
    while (accumulatedTime_ >= clip_.frameDuration) {
        accumulatedTime_ -= clip_.frameDuration;
        ++currentFrame_;
        if (currentFrame_ >= clip_.frameCount) {
            if (clip_.loop) {
                currentFrame_ = 0;
            } else {
                currentFrame_ = clip_.frameCount - 1;
                completed_ = true;
                break;
            }
        }
    }
}

void Animation::Restart() {
    currentFrame_ = 0;
    accumulatedTime_ = 0.0f;
    completed_ = false;
}

bool Animation::IsComplete() const {
    return completed_;
}

Rect Animation::CurrentFrameRect() const {
    const int frameIndex = clip_.firstFrame + currentFrame_;
    return Rect{frameIndex * clip_.frameWidth, 0.0f, clip_.frameWidth, clip_.frameHeight};
}

// The engine's resource managers: own every loaded texture/sound for the
// lifetime of the Engine and deduplicate repeated LoadTexture/LoadSound
// calls for the same path. Deliberately just a vector + a path->index
// cache for each — no per-resource unload, since nothing yet needs it.
// Sounds get their own parallel vector+map rather than sharing the
// texture one: two resource kinds isn't enough evidence to generalize
// the pattern into something shared (see SoundHandle's doc comment).
struct Engine::Impl {
    std::vector<::Texture2D> textures;
    std::unordered_map<std::string, std::size_t> textureIndexByPath;

    std::vector<::Sound> sounds;
    std::unordered_map<std::string, std::size_t> soundIndexByPath;

    ~Impl() {
        for (::Texture2D& texture : textures) {
            UnloadTexture(texture);
        }
        for (::Sound& sound : sounds) {
            UnloadSound(sound);
        }
    }
};

Engine::Engine(const WindowConfig& config) : impl_(std::make_unique<Impl>()) {
    InitWindow(config.width, config.height, config.title);
    SetTargetFPS(config.targetFPS);
    InitAudioDevice();
}

Engine::~Engine() {
    // Textures must be unloaded while the GL context is still alive, and
    // sounds while the audio device is still alive, so destroy the
    // resource managers before tearing down either.
    impl_.reset();
    CloseAudioDevice();
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

float Engine::MouseX() const {
    return ::GetMousePosition().x;
}

float Engine::MouseY() const {
    return ::GetMousePosition().y;
}

bool Engine::IsMouseButtonPressed(MouseButton button) const {
    return ::IsMouseButtonPressed(ToRaylibMouseButton(button));
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

SoundHandle Engine::LoadSound(const char* filePath) {
    const auto existing = impl_->soundIndexByPath.find(filePath);
    if (existing != impl_->soundIndexByPath.end()) {
        return SoundHandle(existing->second);
    }

    impl_->sounds.push_back(::LoadSound(filePath));
    const std::size_t index = impl_->sounds.size() - 1;
    impl_->soundIndexByPath.emplace(filePath, index);
    return SoundHandle(index);
}

void Engine::PlaySound(SoundHandle sound) {
    ::PlaySound(impl_->sounds[sound.index_]);
}

} // namespace engine
