// Focused tests for Bengine's pure, state-free logic -- path resolution and
// camera coordinate conversion. Deliberately hand-rolled (assert-and-count,
// no test framework): the engine itself stays framework-free, and this is
// the first test executable the repo has needed.
//
// Only pure functions are exercised here, via the internal (non-public)
// headers under engine/src/ -- they don't require a live Engine/window, so
// this executable runs headless. Engine::SetAssetRoot's executable-relative
// resolution and Engine::WorldToScreen/ScreenToWorld's raylib-backed
// plumbing are validated separately by running 14_camera_basic (see its
// main.cpp) from different working directories.

#include "engine/Engine.hpp"

#include "../src/CameraMath.hpp"
#include "../src/PathUtil.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

int failureCount = 0;

void Expect(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << "\n";
        ++failureCount;
    }
}

bool NearlyEqual(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) <= epsilon;
}

// Normalizes to forward slashes so the expected strings below don't need to
// vary by platform.
std::string Generic(const std::string& path) {
    return std::filesystem::path(path).generic_string();
}

void TestJoinIfRelative() {
    using engine::detail::JoinIfRelative;

    Expect(Generic(JoinIfRelative("build/Debug", "assets")) == "build/Debug/assets",
           "relative child joins under base");

    Expect(Generic(JoinIfRelative("C:/game/build/Debug", "characters/knight/idle.png")) ==
               "C:/game/build/Debug/characters/knight/idle.png",
           "nested relative child joins under an absolute base");

    Expect(Generic(JoinIfRelative("C:/game/assets", "D:/other/file.png")) == "D:/other/file.png",
           "absolute child bypasses base entirely");

    Expect(JoinIfRelative("", "assets/sprite.png") == "assets/sprite.png",
           "empty base (asset root unset) leaves child unchanged");

    Expect(JoinIfRelative("build/Debug", "") == "",
           "empty child stays empty");
}

void TestCameraMath() {
    using engine::Camera2D;
    using engine::Vec2;
    using engine::detail::CameraScreenToWorld;
    using engine::detail::CameraWorldToScreen;

    constexpr float screenWidth = 800.0f;
    constexpr float screenHeight = 450.0f;

    {
        const Camera2D camera{.position = Vec2{0.0f, 0.0f}, .zoom = 1.0f};
        const Vec2 screen = CameraWorldToScreen(camera, Vec2{0.0f, 0.0f}, screenWidth, screenHeight);
        Expect(NearlyEqual(screen.x, 400.0f) && NearlyEqual(screen.y, 225.0f),
               "world origin under a camera centered on it projects to screen center");
    }

    {
        const Camera2D camera{.position = Vec2{100.0f, 50.0f}, .zoom = 2.0f};
        const Vec2 screen = CameraWorldToScreen(camera, Vec2{110.0f, 50.0f}, screenWidth, screenHeight);
        Expect(NearlyEqual(screen.x, 420.0f) && NearlyEqual(screen.y, 225.0f),
               "zoom scales a world-space offset from the camera position");
    }

    {
        const Camera2D camera{.position = Vec2{123.0f, -45.0f}, .zoom = 1.75f};
        const Vec2 world{200.0f, -300.0f};
        const Vec2 screen = CameraWorldToScreen(camera, world, screenWidth, screenHeight);
        const Vec2 roundTripped = CameraScreenToWorld(camera, screen, screenWidth, screenHeight);
        Expect(NearlyEqual(roundTripped.x, world.x) && NearlyEqual(roundTripped.y, world.y),
               "ScreenToWorld inverts WorldToScreen");
    }
}

} // namespace

int main() {
    TestJoinIfRelative();
    TestCameraMath();

    if (failureCount > 0) {
        std::cerr << failureCount << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
