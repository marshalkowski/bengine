#pragma once

#include <string>
#include <vector>

#include "../utils/serializable.h"

struct AnimatedSpriteRenderer {

	std::vector<std::string> frames;
	int frameIndex;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnimatedSpriteRenderer, frames, frameIndex)