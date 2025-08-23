#pragma once

#include <string>

struct SpriteRenderer {
	std::string path;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteRenderer, path)