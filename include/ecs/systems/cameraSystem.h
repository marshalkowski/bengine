#pragma once

#include <SFML/Graphics.hpp>

#include "../ecs.h"
#include "../system.h"
#include "../components/camera.h"
#include "../components/transform.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class CameraSystem : public System {
public:
	void update(sf::RenderWindow& window) {
		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& camera = ECS::GetComponent<Camera>(entity);
		
			camera.view.setCenter({ transform.position.x, transform.position.y });
			window.setView(camera.view);
		}
	}

	void init(json config) {
		for (auto const& entity : m_entities) {
			auto& camera = ECS::GetComponent<Camera>(entity);

			camera.view = sf::View(sf::FloatRect({ 0.0f,0.0f }, { (float)config["windowSize"]["width"], (float)config["windowSize"]["height"] }));
		}
	}

};