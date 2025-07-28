#pragma once

#include <unordered_set>

#include "../../util/vec2.h"

struct Collider {
	vec2 size;
	bool trigger;
	bool collision;
	std::unordered_set<Entity> entitiesInTrigger;
};