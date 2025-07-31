#pragma once

#include <vector>

#include "../ecs/entity.h"

struct Scene {
	Entity player;
	Entity tileMap;
	std::vector<Entity> otherEntities;
};