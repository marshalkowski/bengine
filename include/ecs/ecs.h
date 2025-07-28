#pragma once

#include "componentManager.h"
#include "entityManager.h"
#include "systemManager.h"

class ECS {
public:
	static ECS& Instance() {
		static ECS instance;
		return instance;
	}

	static Entity CreateEntity() {
		return Instance().m_entityManager->CreateEntity();
	}

	static void DestroyEntity(Entity entity) {
		Instance().m_entityManager->DestroyEntity(entity);
		Instance().m_componentManager->EntityDestroyed(entity);
		Instance().m_systemManager->EntityDestroyed(entity);
	}

	template<typename T>
	static void RegisterComponent() {
		Instance().m_componentManager->RegisterComponent<T>();
	}

	template<typename T>
	static void AddComponent(Entity entity, T component) {
		auto& i = Instance();
		i.m_componentManager->AddComponent<T>(entity, component);

		auto signature = i.m_entityManager->GetSignature(entity);
		signature.set(i.m_componentManager->GetComponentType<T>(), true);
		i.m_entityManager->SetSignature(entity, signature);

		i.m_systemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	static void RemoveComponent(Entity entity) {
		auto& i = Instance();
		i.m_componentManager->RemoveComponent<T>(entity);

		auto signature = i.m_entityManager->GetSignature(entity);
		signature.set(i.m_componentManager->GetComponentType<T>(), false);
		i.m_entityManager->SetSignature(entity, signature);

		i.m_systemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	static T& GetComponent(Entity entity) {
		return Instance().m_componentManager->GetComponent<T>(entity);
	}

	template<typename T>
	static bool HasComponent(Entity entity) {
		return Instance().m_componentManager->HasComponent<T>(entity);
	}

	template<typename T>
	static ComponentType GetComponentType() {
		return Instance().m_componentManager->GetComponentType<T>();
	}

	template<typename T>
	static std::shared_ptr<T> RegisterSystem() {
		return Instance().m_systemManager->RegisterSystem<T>();
	}

	template<typename T>
	static void SetSystemSignature(Signature signature) {
		Instance().m_systemManager->SetSignature<T>(signature);
	}


private:
	ECS() {
		m_componentManager = std::make_unique<ComponentManager>();
		m_entityManager = std::make_unique<EntityManager>();
		m_systemManager = std::make_unique<SystemManager>();
	}

	std::unique_ptr<ComponentManager> m_componentManager;
	std::unique_ptr<EntityManager> m_entityManager;
	std::unique_ptr<SystemManager> m_systemManager;
};