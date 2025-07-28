#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include "../system.h"
#include "../components/collider.h"
#include "../components/movable.h"
#include "../components/transform.h"
#include "../ecs.h"

#include "../../events/collisionEvent.h"
#include "../../events/triggerEvents.h"

struct GridCoord {
	int x, y;
	bool operator==(const GridCoord& other) const {
		return x == other.x && y == other.y;
	}
};

struct GridHash {
	size_t operator()(const GridCoord& c) const {
		return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1);
	}
};

class CollisionSystem : public System {
public:
	void update(float dt) {
		m_spatialMap.clear();
		
		broadPhase();
		narrowPhase(dt);
	}

	void debug(sf::RenderWindow& window) {
		for (auto cell : m_spatialMap) {
			sf::RectangleShape rect({ CELL_SIZE - 4.0f, CELL_SIZE - 4.0f });
			rect.setPosition({ cell.first.x * CELL_SIZE +2.0f,cell.first.y * CELL_SIZE +2.0f });
			rect.setOutlineColor(sf::Color::White);
			rect.setOutlineThickness(1.0f);
			rect.setFillColor(sf::Color::Transparent);
			window.draw(rect);
		}

		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& collider = ECS::GetComponent<Collider>(entity);

			auto const size = collider.size;

			sf::RectangleShape rect({ (float)size.x,(float)size.y });
			rect.setPosition({ transform.position.x - size.x / 2.0f, transform.position.y - size.y / 2.0f });
			rect.setOutlineColor(collider.collision || collider.entitiesInTrigger.size() > 0 ? sf::Color::Red : sf::Color::Green);
			rect.setOutlineThickness(1.0f);
			rect.setFillColor(sf::Color::Transparent);
			window.draw(rect);
		}
	}

	

private:
	void broadPhase() {
		// bucket entities into spatial grid
		for (auto const& entity : m_entities) {
			auto position = ECS::GetComponent<Transform>(entity).position;

			m_spatialMap[positionToGridCoord(position)].push_back(entity);
		}
	}

	GridCoord positionToGridCoord(vec2 position) {
		int gridX = std::floor(position.x / CELL_SIZE);
		int gridY = std::floor(position.y / CELL_SIZE);

		return { gridX, gridY };
	}

	void narrowPhase(float dt) {
		for (auto const& entity : m_entities) {
			ECS::GetComponent<Collider>(entity).collision = false;
		}
		for (auto const& entity : m_entities) {
			if (ECS::HasComponent<Movable>(entity) && ECS::GetComponent<Movable>(entity).delta.Length() != 0) {

				auto nearbyEntities = querySpatialGrid(entity);
				for (auto& other : nearbyEntities) {
					if (other == entity) continue;

					checkCollision(entity, other, dt);
				}
			}
			else {
				auto& collider = ECS::GetComponent<Collider>(entity);
				if (collider.trigger && collider.entitiesInTrigger.size() > 0) {
					std::vector<Entity> entitiesToRemove = {};
					for (auto& other : collider.entitiesInTrigger) {
						if (!checkCollision(other, entity, dt)) {
							entitiesToRemove.push_back(other);
						}
					}
					for (auto e : entitiesToRemove) {
						const TriggerExitEvent event(entity, e);
						Global::eventBus().emit<TriggerExitEvent>(event);
						collider.entitiesInTrigger.erase(e);
					}
				}
			}
		}
	}

	bool checkCollision(Entity entity, Entity other, float dt) {
		auto& entityMovable = ECS::GetComponent<Movable>(entity);
		auto entityTrigger = ECS::GetComponent<Collider>(entity).trigger;
		auto otherTrigger = ECS::GetComponent<Collider>(other).trigger;
		vec2 entityXPosition = ECS::GetComponent<Transform>(entity).position +
			vec2(entityMovable.delta.x, 0.0f) * entityMovable.speed * dt;
		vec2 entityYPosition = ECS::GetComponent<Transform>(entity).position +
			vec2(0.0f, entityMovable.delta.y) * entityMovable.speed * dt;
		vec2 otherPosition = ECS::GetComponent<Transform>(other).position;

		bool collisionXAxis = false;
		bool collisionYAxis = false;

		// try x Move
		if (AABB(entity, other, entityXPosition, otherPosition)) {
			collisionXAxis = true;
			//if (entityTrigger || otherTrigger) {
			//	auto& otherCollider = ECS::GetComponent<Collider>(other);
			//	otherCollider.entitiesInTrigger.insert(entity);
			//}
			ECS::GetComponent<Collider>(entity).collision = true;
		}

		// try y Move
		if (AABB(entity, other, entityYPosition, otherPosition)) {
			collisionYAxis = true;
			//if (entityTrigger || otherTrigger) {
			//	auto& otherCollider = ECS::GetComponent<Collider>(other);
			//	otherCollider.entitiesInTrigger.insert(entity);
			//}
			ECS::GetComponent<Collider>(entity).collision = true;
		}

		if (!collisionXAxis && !collisionYAxis) {
			return false;
		}
		if (!entityTrigger && !otherTrigger) {
			const CollisionEvent e(entity, other, collisionXAxis, collisionYAxis);
			Global::eventBus().emit<CollisionEvent>(e);
		}
		else if (otherTrigger) {
			auto& otherCollider = ECS::GetComponent<Collider>(other);
			auto& existingTriggers = otherCollider.entitiesInTrigger;
			if (existingTriggers.find(entity) == existingTriggers.end()) {
				const TriggerEnterEvent e(other, entity);
				Global::eventBus().emit<TriggerEnterEvent>(e);
				existingTriggers.insert(entity);
			}
			else {
				const TriggerStayEvent e(other, entity);
				Global::eventBus().emit<TriggerStayEvent>(e);
			}
		}

		return collisionXAxis || collisionYAxis;
	}

	std::unordered_set<Entity> querySpatialGrid(Entity entity) {
		std::unordered_set<Entity> result;

		auto position = ECS::GetComponent<Transform>(entity).position;
		auto mapPosition = positionToGridCoord(position);
		for (auto x = -1; x <= 1; x++) {
			for (auto y = -1; y <= 1; y++) {
				GridCoord coord{ mapPosition.x + x, mapPosition.y + y };
				auto it = m_spatialMap.find(coord);
				if (it != m_spatialMap.end()) {
					for (Entity other : it->second) {
						result.insert(other);
					}
				}
			}
		}

		return result;
	}

	bool AABB(Entity a, Entity b, vec2 posA, vec2 posB) {
		auto& tA = ECS::GetComponent<Transform>(a);
		auto& cA = ECS::GetComponent<Collider>(a);
		auto& tB = ECS::GetComponent<Transform>(b);
		auto& cB = ECS::GetComponent<Collider>(b);

		if (posA.x - cA.size.x / 2.0f > posB.x + cB.size.x / 2.0f || // A is to right of B
			posB.x - cB.size.x / 2.0f > posA.x + cA.size.x / 2.0f) { // B is to right of A
			return false;
		}

		if (posA.y - cA.size.y / 2.0f > posB.y + cB.size.y / 2.0f || // A is below B
			posB.y - cB.size.y / 2.0f > posA.y + cA.size.y / 2.0f) { // B is below A
			return false;
		}

		return true;

	}

	std::unordered_map<Entity, vec2> m_staticEntities;
	std::unordered_map<Entity, vec2> m_dynamicEntities;

	std::unordered_map<GridCoord, std::vector<Entity>, GridHash> m_spatialMap;

	const float CELL_SIZE = 32.0f;
};