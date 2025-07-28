#pragma once

#include <array>
#include <unordered_map>
#include <assert.h>

#include "entity.h"

class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	virtual void EntityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
public:
	void InsertData(Entity entity, T component) {
		assert(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() && "Component added to entity more than once");

		size_t newIndex = m_size;
		m_entityToIndexMap[entity] = newIndex;
		m_indexToEntityMap[newIndex] = entity;
		m_componentArray[newIndex] = component;
		++m_size;
	}

	void RemoveData(Entity entity) {
		assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && "Removing non-existent component");

		size_t indexOfRemovedEntity = m_entityToIndexMap[entity];
		size_t indexOfLastElement = m_size - 1;
		m_componentArray[indexOfRemovedEntity] = m_componentArray[indexOfLastElement];

		Entity entityOfLastElement = m_indexToEntityMap[indexOfLastElement];
		m_entityToIndexMap[indexOfLastElement] = indexOfRemovedEntity;
		m_indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

		m_entityToIndexMap.erase(entity);
		m_indexToEntityMap.erase(indexOfLastElement);

		--m_size;
	}

	T& GetData(Entity entity) {
		assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() && "Retrieving non-existent component");

		return m_componentArray[m_entityToIndexMap[entity]];
	}

	bool EntityExists(Entity entity) {
		return m_entityToIndexMap.find(entity) != m_entityToIndexMap.end();
	}

	void EntityDestroyed(Entity entity) override {
		if (m_entityToIndexMap.find(entity) != m_entityToIndexMap.end()) {
			RemoveData(entity);
		}
	}


private:
	std::array<T, MAX_ENTITIES> m_componentArray;

	std::unordered_map<Entity, size_t> m_entityToIndexMap;

	std::unordered_map<size_t, Entity> m_indexToEntityMap;

	size_t m_size;
};