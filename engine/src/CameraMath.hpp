#pragma once

// Internal helper, not part of Bengine's public API (lives under src/, not
// include/). Pulled out of Engine.cpp so the coordinate-conversion math can
// be unit-tested directly without needing a live Engine/window. Matches
// what Engine::BeginCameraMode hands raylib (offset = screen center,
// rotation = 0) so results here are consistent with what's actually drawn.

#include "engine/Engine.hpp"

namespace engine::detail {

inline Vec2 CameraWorldToScreen(const Camera2D& camera, Vec2 worldPoint,
                                 float screenWidth, float screenHeight) {
    return Vec2{
        (worldPoint.x - camera.position.x) * camera.zoom + screenWidth * 0.5f,
        (worldPoint.y - camera.position.y) * camera.zoom + screenHeight * 0.5f,
    };
}

inline Vec2 CameraScreenToWorld(const Camera2D& camera, Vec2 screenPoint,
                                 float screenWidth, float screenHeight) {
    return Vec2{
        (screenPoint.x - screenWidth * 0.5f) / camera.zoom + camera.position.x,
        (screenPoint.y - screenHeight * 0.5f) / camera.zoom + camera.position.y,
    };
}

} // namespace engine::detail
