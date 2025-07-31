#pragma once

#include <vector>
#include <iostream>

#include "../ecs/entity.h"
#include "scene.h"
#include "demoScene.h"

class SceneManager {
public:
	void LoadDemoScene(json config) {
		m_scene = std::make_unique<Scene>(DemoScene::Create(config));

		std::cout << m_scene->otherEntities.size();
	}

	void ClearScene() {
		ECS::DestroyEntity(m_scene->player);
		ECS::DestroyEntity(m_scene->tileMap);
		while (m_scene->otherEntities.size() > 0) {
			std::cout << m_scene->otherEntities.size();
			ECS::DestroyEntity(m_scene->otherEntities[0]);
			m_scene->otherEntities.erase(m_scene->otherEntities.begin());
		}
		m_scene = nullptr;
	}


private:
	std::unique_ptr<Scene> m_scene;
};