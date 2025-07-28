#pragma once

#include "../system.h"
#include "../ecs.h"
#include "../components/movable.h"
#include "../components/followMovement.h"
#include "../../input.h"
#include "../../util/vec2.h"

class FollowMoveSystem : public System {
public:
	void update() {
		for (auto const& entity : m_entities) {
			auto& movable = ECS::GetComponent<Movable>(entity);
			auto& followMove = ECS::GetComponent<FollowMovement>(entity);
			auto& transform = ECS::GetComponent<Transform>(entity);

			auto& targetTransform = ECS::GetComponent<Transform>(followMove.target);

			auto delta = vec2(targetTransform.position.x - transform.position.x,
				targetTransform.position.y - transform.position.y);

			delta = delta.Normalized();

			movable.delta = delta;
		}
	}
};