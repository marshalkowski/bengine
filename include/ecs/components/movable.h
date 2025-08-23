#pragma once

struct Movable {
	vec2 delta;
	float speed;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Movable, delta, speed)