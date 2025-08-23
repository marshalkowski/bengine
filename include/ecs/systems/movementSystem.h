#pragma once

#include <iostream>

#include "../system.h"
#include "../ecs.h"
#include "../components/transform.h"
#include "../components/movable.h"
#include "../../input.h"

#include "../../events/collisionEvent.h"

class MovementSystem : public System {
public:
	void update(float dt) {
		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& movable = ECS::GetComponent<Movable>(entity);

			transform.position = vec2(
				transform.position.x + (movable.delta.x * dt * movable.speed), 
				transform.position.y + (movable.delta.y * dt * movable.speed)
			);

			auto& sceneObject = ECS::GetComponent<SceneObject>(entity);
			for (auto const& childEntity : sceneObject.children) {
				if (ECS::HasComponent<Transform>(childEntity)) {
					auto& childTransform = ECS::GetComponent<Transform>(childEntity);
					childTransform.position = vec2(
						childTransform.position.x + (movable.delta.x * dt * movable.speed),
						childTransform.position.y + (movable.delta.y * dt * movable.speed)
					);
				}
			}
		}
	}

	void init() {
		Global::eventBus().subscribe<CollisionEvent>([](const CollisionEvent& e) {
			if (ECS::HasComponent<Movable>(e.a)) {
				auto& movable = ECS::GetComponent<Movable>(e.a);
				if (e.xAxis) {
					movable.delta.x = 0.0f;
				}
				if (e.yAxis) {
					movable.delta.y = 0.0f;
				}
			}
		});

		if (Global::isDebug()) {
			Global::eventBus().subscribe<TriggerEnterEvent>([](const TriggerEnterEvent& e) {
				std::cout << e.entity << " entered " << e.trigger << "\n";
			});

			Global::eventBus().subscribe<TriggerStayEvent>([](const TriggerStayEvent& e) {
				std::cout << e.entity << " is in " << e.trigger << "\n";
			});

			Global::eventBus().subscribe<TriggerExitEvent>([](const TriggerExitEvent& e) {
				std::cout << e.entity << " exited " << e.trigger << "\n";
			});
		}
	}
};