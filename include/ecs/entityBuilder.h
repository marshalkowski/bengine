#pragma once

#include <assert.h>

#include "ecs.h"
#include "components/animatedSpriteRenderer.h"
#include "components/collider.h"
#include "components/transform.h"
#include "components/movable.h"
#include "components/followMovement.h"
#include "components/inputMovement.h"
#include "components/spriteRenderer.h"
#include "components/tileMapRenderer.h"
#include "../util/vec2.h"

class EntityBuilder {

public:
	EntityBuilder& CreateEntity() {
		init = true;
		entity = ECS::CreateEntity();
		return *this;
	}

	EntityBuilder& AddTransform(vec2 position) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			Transform{ position }
		);

		return *this;
	}

	EntityBuilder& AddSprite(std::string filePath) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			SpriteRenderer{ filePath }
		);

		return *this;
	}

	EntityBuilder& AddAnimatedSprite(std::vector<std::string> frames) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			AnimatedSpriteRenderer{ frames, 0 }
		);

		return *this;
	}

	EntityBuilder& AddMovable(float speed) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			Movable{ vec2(0,0), speed }
		);

		return *this;
	}

	EntityBuilder& AddFollowMovement(Entity target) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			FollowMovement{ target }
		);

		return *this;
	}

	EntityBuilder& AddInputMovement() {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			InputMovement{}
		);

		return *this;
	}

	EntityBuilder& AddCollider(vec2 size, bool trigger = false) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			Collider{ size, trigger }
		);

		return *this;
	}

	EntityBuilder& AddTileMap(TileMap tileMap) {
		assert(init && "entity must be created");

		ECS::AddComponent(
			entity,
			TileMapRenderer{tileMap}
		);

		return *this;
	}

	Entity ID() {
		return entity;
	}

private:
	bool init = false;
	Entity entity;

};