#pragma once

#include "../ecs/components/sceneObject.h"
#include "../ecs/ecs.h"
#include "../ecs/entity.h"

class SceneUtils {
public:
	static void ParentChild(Entity parent, Entity child, Scene& scene) {
		auto& pso = ECS::GetComponent<SceneObject>(parent);
		auto& cso = ECS::GetComponent<SceneObject>(child);
		pso.children.insert(child);
		cso.parent = parent;
		RemoveFromScene(child, scene);
	}

	// adds entity to root level of scene
	static void AddEntityToScene(Entity entity, Scene& scene) {
		scene.entities.push_back(entity);
	}

	static void RemoveFromScene(Entity entity, Scene& scene) {
		scene.entities.erase(std::remove(scene.entities.begin(), scene.entities.end(), entity), scene.entities.end());
	}

};