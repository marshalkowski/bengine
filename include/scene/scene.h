#pragma once

#include <vector>

#include "../ecs/entity.h"

struct Scene {

	Scene() {
		entities = std::vector<uint32_t>();
	}

	std::vector<uint32_t> entities;
};