#pragma once

#include <string>
#include <unordered_set>

#include "../entity.h"

struct SceneObject {
	std::string name;
	Entity parent;
	std::unordered_set<Entity> children;
};