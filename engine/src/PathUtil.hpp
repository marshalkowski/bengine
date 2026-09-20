#pragma once

// Internal helper, not part of Bengine's public API (lives under src/, not
// include/). Pulled out of Engine.cpp so it can be unit-tested directly
// without needing a live Engine/window.

#include <filesystem>
#include <string>

namespace engine::detail {

// If `child` is empty, or `base` is empty, or `child` is already an
// absolute path, returns `child` unchanged. Otherwise returns `base`
// joined with `child`. Used for both path resolutions Engine::SetAssetRoot/
// ResolveAssetPath need, with the same rule applied twice: resolving the
// asset root itself against the executable directory, and resolving an
// individual resource path against the asset root.
inline std::string JoinIfRelative(const std::string& base, const std::string& child) {
    if (base.empty() || child.empty() || std::filesystem::path(child).is_absolute()) {
        return child;
    }
    return (std::filesystem::path(base) / child).string();
}

} // namespace engine::detail
