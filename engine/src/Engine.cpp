#include "engine/Engine.hpp"

#include <cstdio>
#include <unordered_map>
#include <utility>
#include <vector>

#include "raylib.h"

#include "CameraMath.hpp"
#include "PathUtil.hpp"

namespace {

::Color ToRaylibColor(engine::Color color) {
    return {color.r, color.g, color.b, color.a};
}

int ToRaylibKey(engine::Key key) {
    switch (key) {
        case engine::Key::A: return KEY_A;
        case engine::Key::B: return KEY_B;
        case engine::Key::C: return KEY_C;
        case engine::Key::D: return KEY_D;
        case engine::Key::E: return KEY_E;
        case engine::Key::F: return KEY_F;
        case engine::Key::G: return KEY_G;
        case engine::Key::H: return KEY_H;
        case engine::Key::I: return KEY_I;
        case engine::Key::J: return KEY_J;
        case engine::Key::K: return KEY_K;
        case engine::Key::L: return KEY_L;
        case engine::Key::M: return KEY_M;
        case engine::Key::N: return KEY_N;
        case engine::Key::O: return KEY_O;
        case engine::Key::P: return KEY_P;
        case engine::Key::Q: return KEY_Q;
        case engine::Key::R: return KEY_R;
        case engine::Key::S: return KEY_S;
        case engine::Key::T: return KEY_T;
        case engine::Key::U: return KEY_U;
        case engine::Key::V: return KEY_V;
        case engine::Key::W: return KEY_W;
        case engine::Key::X: return KEY_X;
        case engine::Key::Y: return KEY_Y;
        case engine::Key::Z: return KEY_Z;
        case engine::Key::Zero: return KEY_ZERO;
        case engine::Key::One: return KEY_ONE;
        case engine::Key::Two: return KEY_TWO;
        case engine::Key::Three: return KEY_THREE;
        case engine::Key::Four: return KEY_FOUR;
        case engine::Key::Five: return KEY_FIVE;
        case engine::Key::Six: return KEY_SIX;
        case engine::Key::Seven: return KEY_SEVEN;
        case engine::Key::Eight: return KEY_EIGHT;
        case engine::Key::Nine: return KEY_NINE;
        case engine::Key::Up: return KEY_UP;
        case engine::Key::Down: return KEY_DOWN;
        case engine::Key::Left: return KEY_LEFT;
        case engine::Key::Right: return KEY_RIGHT;
        case engine::Key::Space: return KEY_SPACE;
        case engine::Key::Enter: return KEY_ENTER;
        case engine::Key::Escape: return KEY_ESCAPE;
        case engine::Key::Tab: return KEY_TAB;
        case engine::Key::Backspace: return KEY_BACKSPACE;
        case engine::Key::LeftShift: return KEY_LEFT_SHIFT;
        case engine::Key::RightShift: return KEY_RIGHT_SHIFT;
        case engine::Key::LeftCtrl: return KEY_LEFT_CONTROL;
        case engine::Key::RightCtrl: return KEY_RIGHT_CONTROL;
        case engine::Key::LeftAlt: return KEY_LEFT_ALT;
        case engine::Key::RightAlt: return KEY_RIGHT_ALT;
    }
    return KEY_NULL;
}

int ToRaylibMouseButton(engine::MouseButton button) {
    switch (button) {
        case engine::MouseButton::Left: return MOUSE_BUTTON_LEFT;
    }
    return MOUSE_BUTTON_LEFT;
}

// offset = screen center, rotation = 0 -- kept in sync with the pure
// conversion math in CameraMath.hpp (see WorldToScreen/ScreenToWorld below).
::Camera2D ToRaylibCamera2D(const engine::Camera2D& camera) {
    return ::Camera2D{
        ::Vector2{static_cast<float>(::GetScreenWidth()) * 0.5f, static_cast<float>(::GetScreenHeight()) * 0.5f},
        ::Vector2{camera.position.x, camera.position.y},
        0.0f,
        camera.zoom,
    };
}

} // namespace

namespace engine {

std::string ToString(float value, int decimalPlaces) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", decimalPlaces, value);
    return std::string(buffer);
}

std::string ExecutableDirectory() {
    return ::GetApplicationDirectory();
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

    // Empty means unset -- LoadTexture/LoadSound/ResolveAssetPath then leave
    // paths untouched (see Engine::SetAssetRoot).
    std::string assetRoot;

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

void Engine::BeginCameraMode(const Camera2D& camera) {
    ::BeginMode2D(ToRaylibCamera2D(camera));
}

void Engine::EndCameraMode() {
    ::EndMode2D();
}

Vec2 Engine::WorldToScreen(const Camera2D& camera, Vec2 worldPoint) const {
    return detail::CameraWorldToScreen(camera, worldPoint,
                                        static_cast<float>(::GetScreenWidth()),
                                        static_cast<float>(::GetScreenHeight()));
}

Vec2 Engine::ScreenToWorld(const Camera2D& camera, Vec2 screenPoint) const {
    return detail::CameraScreenToWorld(camera, screenPoint,
                                        static_cast<float>(::GetScreenWidth()),
                                        static_cast<float>(::GetScreenHeight()));
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

void Engine::SetAssetRoot(const std::string& root) {
    impl_->assetRoot = root.empty() ? std::string() : detail::JoinIfRelative(ExecutableDirectory(), root);
}

std::string Engine::ResolveAssetPath(const std::string& relativePath) const {
    return detail::JoinIfRelative(impl_->assetRoot, relativePath);
}

TextureHandle Engine::LoadTexture(const char* filePath) {
    const std::string resolvedPath = ResolveAssetPath(filePath);

    const auto existing = impl_->textureIndexByPath.find(resolvedPath);
    if (existing != impl_->textureIndexByPath.end()) {
        return TextureHandle(existing->second);
    }

    impl_->textures.push_back(::LoadTexture(resolvedPath.c_str()));
    const std::size_t index = impl_->textures.size() - 1;
    impl_->textureIndexByPath.emplace(resolvedPath, index);
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
    const std::string resolvedPath = ResolveAssetPath(filePath);

    const auto existing = impl_->soundIndexByPath.find(resolvedPath);
    if (existing != impl_->soundIndexByPath.end()) {
        return SoundHandle(existing->second);
    }

    impl_->sounds.push_back(::LoadSound(resolvedPath.c_str()));
    const std::size_t index = impl_->sounds.size() - 1;
    impl_->soundIndexByPath.emplace(resolvedPath, index);
    return SoundHandle(index);
}

void Engine::PlaySound(SoundHandle sound) {
    ::PlaySound(impl_->sounds[sound.index_]);
}

} // namespace engine
