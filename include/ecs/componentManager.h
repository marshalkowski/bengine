#pragma once

#include <unordered_map>
#include <memory>
#include <assert.h>

#include "entity.h"
#include "componentArray.h"

class ComponentManager {
public:
	template<typename T>
	void RegisterComponent() {
		const char* typeName = typeid(T).name();

		assert(m_componentTypes.find(typeName) == m_componentTypes.end() && "Component already registered");

		m_componentTypes.insert({ typeName, m_nextComponentType });

		m_componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

		++m_nextComponentType;
	}

	template<typename T>
	ComponentType GetComponentType() {
		const char* typeName = typeid(T).name();
		
		assert(m_componentTypes.find(typeName) != m_componentTypes.end() && "Component not registered");

		return m_componentTypes[typeName];
	}

	template<typename T>
	void AddComponent(Entity entity, T component) {
		GetComponentArray<T>()->InsertData(entity, component);
	}

	template<typename T>
	void RemoveComponent(Entity entity) {
		GetComponentArray<T>()->RemoveData(entity);
	}

	template<typename T>
	T& GetComponent(Entity entity) {
		return GetComponentArray<T>()->GetData(entity);
	}

	template<typename T>
	bool HasComponent(Entity entity) {
		return GetComponentArray<T>()->EntityExists(entity);
	}

	void EntityDestroyed(Entity entity) {
		for (auto const& pair : m_componentArrays) {
			auto const& component = pair.second;

			component->EntityDestroyed(entity);
		}
	}

	std::unordered_map<const char*, ComponentType> GetComponentTypes() {
		return m_componentTypes;
	}

private:
	std::unordered_map<const char*, ComponentType> m_componentTypes{};

	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_componentArrays{};

	ComponentType m_nextComponentType{};

	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray() {
		const char* typeName = typeid(T).name();

		assert(m_componentTypes.find(typeName) != m_componentTypes.end() && "Component not registered");

		return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays[typeName]);
	}
};