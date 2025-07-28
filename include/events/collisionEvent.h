#pragma once

#include "../ecs/entity.h"

struct CollisionEvent {
	Entity a;
	Entity b;
	bool xAxis;
	bool yAxis;

	CollisionEvent(Entity a, Entity b, bool x, bool y) : a(a), b(b), xAxis(x), yAxis(y) {};
};