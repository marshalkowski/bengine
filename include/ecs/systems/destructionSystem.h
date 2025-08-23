#pragma once

#include "../../global.h"
#include "../components/destructible.h"
#include "../components/sceneObject.h"
#include "../system.h"
#include "../ecs.h"
#include "../../scene/sceneManager.h"

#include "../../events/collisionEvent.h"

class DestructionSystem : public System {

public:
	void init() {
		Global::eventBus().subscribe<CollisionEvent>([](const CollisionEvent& e) {
			if (ECS::HasComponent<Destructible>(e.b) && ECS::GetComponent<SceneObject>(e.a).name == "Player") {
				SceneManager::RemoveObject(e.b);
			}
		});
	}


};