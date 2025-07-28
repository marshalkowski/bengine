#pragma once

#include "../system.h"
#include "../ecs.h"
#include "../components/movable.h"
#include "../../input.h"
#include "../../util/vec2.h"

class InputMoveSystem : public System {
public:
	void update() {
		for (auto const& entity : m_entities) {
			auto& movable = ECS::GetComponent<Movable>(entity);

			vec2 delta;

			if (Input::up()) {
				delta.y += -1;
			}
			if (Input::down()) {
				delta.y += 1;
			}
			if (Input::left()) {
				delta.x += -1;
			}
			if (Input::right()) {
				delta.x += 1;
			}

			delta = delta.Normalized();

			movable.delta = delta;
		}
	}
};