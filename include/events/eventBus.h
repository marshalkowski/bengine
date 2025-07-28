#pragma once

#include <functional>
#include <unordered_map>
#include <typeindex>

class EventBus {

public:
	template <typename T>
	void subscribe(std::function<void(const T&)> listener) {
		auto& listeners = getListeners<T>();
		listeners.push_back(std::move(listener));
	}

	template <typename T>
	void emit(const T& event) {
		for (auto& listener : getListeners<T>()) {
			listener(event);
		}
	}

	void clear() {
		subscribers.clear();
	}

private:
	std::unordered_map<std::type_index, std::vector<std::function<void(void*)>>> subscribers;

	template <typename T>
	std::vector<std::function<void(const T&)>>& getListeners() {
		auto typeId = std::type_index(typeid(T));

		if (subscribers.find(typeId) == subscribers.end()) {
			subscribers[typeId] = {};
		}

		auto& rawListeners = subscribers[typeId];

		if constexpr (!std::is_same_v<decltype(typedListeners<T>), void>) {
			return typedListeners<T>;
		}
		else {
			typedListeners<T> = std::vector<std::function<void(const T&)>>{};
			return typedListeners<T>;
		}
	}

	template <typename T>
	static inline std::vector<std::function<void(const T&)>> typedListeners;

};