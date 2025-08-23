#pragma once

#include <vector>
#include <iostream>

#include "../ecs/entity.h"
#include "../ecs/ecssystems.h"
#include "scene.h"
#include "demoScene.h"
#include "../global.h"

class SceneManager {
public:
	static SceneManager& Instance() {
		static SceneManager instance;
		return instance;
	}

	static void LoadDemoScene(json config) {
		Instance().m_scene = std::make_unique<Scene>(DemoScene::Create(config));
		ECSSystems::Cameras()->init(Global::config());
	}

	static void Init() {
		Instance().InitScene();
	}

	void InitScene() {
		m_scene = std::make_unique<Scene>();
	}

	static void ClearScene() {
		if (!IsSceneLoaded()) {
			return;
		}
		while (Instance().m_scene->entities.size() > 0) {
			ECS::DestroyEntity(Instance().m_scene->entities[0]);
			Instance().m_scene->entities.erase(Instance().m_scene->entities.begin());
		}
		Instance().m_scene = nullptr;
	}

	static bool IsSceneLoaded() {
		return Instance().m_scene != nullptr;
	}

	static Scene& GetScene() {
		return *(Instance().m_scene.get());
	}

	static void AddObject(Entity e) {
		if (Instance().m_scene == nullptr) {
			Instance().InitScene();
		}
		SceneUtils::AddEntityToScene(e, *(Instance().m_scene));
	}

	static void RemoveObject(Entity e) {
		if (Instance().m_scene == nullptr) {
			Instance().InitScene();
		}
		SceneUtils::RemoveFromScene(e, *Instance().m_scene);
	}

	static void ParentChild(Entity parent, Entity child) {
		SceneUtils::ParentChild(parent, child, *Instance().m_scene);
	}

	static Entity GetObjectByName(const std::string& name) {
		if (!IsSceneLoaded) {
			return -1;
		}
		
		for (auto e : Instance().m_scene->entities) {
			if (ECS::GetComponent<SceneObject>(e).name == name) {
				return e;
			}
		}
		return -1;
	}

private:
	std::unique_ptr<Scene> m_scene;
};