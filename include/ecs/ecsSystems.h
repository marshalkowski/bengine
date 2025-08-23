#pragma once

#include <memory>

#include "ecs.h"

#include "components/animatedSpriteRenderer.h"
#include "components/camera.h"
#include "components/collider.h"
#include "components/followMovement.h"
#include "components/inputMovement.h"
#include "components/movable.h"
#include "components/sceneObject.h"
#include "components/spriteRenderer.h"
#include "components/tileMapRenderer.h"
#include "components/transform.h"

#include "systems/animatedSpriteRenderSystem.h"
#include "systems/cameraSystem.h"
#include "systems/collisionSystem.h"
#include "systems/followMoveSystem.h"
#include "systems/inputMoveSystem.h"
#include "systems/movementSystem.h"
#include "systems/spriteRenderSystem.h"
#include "systems/tileMapRenderSystem.h"

class ECSSystems {
public:
	static ECSSystems& Instance() {
		static ECSSystems instance;
		return instance;
	}
	
	static void Init() {
		ECS::RegisterComponent<AnimatedSpriteRenderer>();
		ECS::RegisterComponent<Camera>();
		ECS::RegisterComponent<Collider>();
		ECS::RegisterComponent<FollowMovement>();
		ECS::RegisterComponent<InputMovement>();
		ECS::RegisterComponent<Movable>();
		ECS::RegisterComponent<SceneObject>();
		ECS::RegisterComponent<SpriteRenderer>();
		ECS::RegisterComponent<TileMapRenderer>();
		ECS::RegisterComponent<Transform>();

		Instance().m_animatedSpriteRenderer = ECS::RegisterSystem<AnimatedSpriteRenderSystem>();
		Signature animSpriteSignature;
		animSpriteSignature.set(ECS::GetComponentType<AnimatedSpriteRenderer>());
		animSpriteSignature.set(ECS::GetComponentType<Transform>());
		ECS::SetSystemSignature<AnimatedSpriteRenderSystem>(animSpriteSignature);

		Instance().m_camera = ECS::RegisterSystem<CameraSystem>();
		Signature cameraSignature;
		cameraSignature.set(ECS::GetComponentType<Camera>());
		cameraSignature.set(ECS::GetComponentType<Transform>());
		ECS::SetSystemSignature<CameraSystem>(cameraSignature);

		Instance().m_collision = ECS::RegisterSystem<CollisionSystem>();
		Signature collisionSignature;
		collisionSignature.set(ECS::GetComponentType<Transform>());
		collisionSignature.set(ECS::GetComponentType<Collider>());
		ECS::SetSystemSignature<CollisionSystem>(collisionSignature);

		Instance().m_followMove = ECS::RegisterSystem<FollowMoveSystem>();
		Signature followMoveSignature;
		followMoveSignature.set(ECS::GetComponentType<FollowMovement>());
		followMoveSignature.set(ECS::GetComponentType<Movable>());
		followMoveSignature.set(ECS::GetComponentType<Transform>());
		ECS::SetSystemSignature<FollowMoveSystem>(followMoveSignature);

		Instance().m_inputMove = ECS::RegisterSystem<InputMoveSystem>();
		Signature inputMoveSignature;
		inputMoveSignature.set(ECS::GetComponentType<InputMovement>());
		inputMoveSignature.set(ECS::GetComponentType<Movable>());
		ECS::SetSystemSignature<InputMoveSystem>(inputMoveSignature);

		Instance().m_movement = ECS::RegisterSystem<MovementSystem>();
		Signature movementSignature;
		movementSignature.set(ECS::GetComponentType<Transform>());
		movementSignature.set(ECS::GetComponentType<Movable>());
		ECS::SetSystemSignature<MovementSystem>(movementSignature);
		Instance().m_movement->init();

		Instance().m_spriteRenderer = ECS::RegisterSystem<SpriteRenderSystem>();
		Signature spriteSignature;
		spriteSignature.set(ECS::GetComponentType<Transform>());
		spriteSignature.set(ECS::GetComponentType<SpriteRenderer>());
		ECS::SetSystemSignature<SpriteRenderSystem>(spriteSignature);

		Instance().m_tileMapRenderer = ECS::RegisterSystem<TileMapRenderSystem>();
		Signature tileMapSignature;
		tileMapSignature.set(ECS::GetComponentType<TileMapRenderer>());
		ECS::SetSystemSignature<TileMapRenderSystem>(tileMapSignature);
	}

	static std::shared_ptr<AnimatedSpriteRenderSystem> AnimatedSpriteRender() {
		return Instance().m_animatedSpriteRenderer;
	}

	static std::shared_ptr<CameraSystem> Cameras() {
		return Instance().m_camera;
	}

	static std::shared_ptr<CollisionSystem> Collision() {
		return Instance().m_collision;
	}

	static std::shared_ptr<FollowMoveSystem> FollowMove() {
		return Instance().m_followMove;
	}

	static std::shared_ptr<InputMoveSystem> InputMove() {
		return Instance().m_inputMove;
	}

	static std::shared_ptr<MovementSystem> Movement() {
		return Instance().m_movement;
	}

	static std::shared_ptr<SpriteRenderSystem> SpriteRender() {
		return Instance().m_spriteRenderer;
	}

	static std::shared_ptr<TileMapRenderSystem> TileMapRender() {
		return Instance().m_tileMapRenderer;
	}

private:
	std::shared_ptr<AnimatedSpriteRenderSystem> m_animatedSpriteRenderer;
	std::shared_ptr<CameraSystem> m_camera;
	std::shared_ptr<CollisionSystem> m_collision;
	std::shared_ptr<FollowMoveSystem> m_followMove;
	std::shared_ptr<InputMoveSystem> m_inputMove;
	std::shared_ptr<MovementSystem> m_movement;
	std::shared_ptr<SpriteRenderSystem> m_spriteRenderer;
	std::shared_ptr<TileMapRenderSystem> m_tileMapRenderer;
};