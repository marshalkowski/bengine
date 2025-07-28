#pragma once

#include <SFML/System.hpp>

class Chrono {
public:
	static float deltaTime() {
		return m_deltaTime;
	}

	static void init() {
		m_clock.start();
	}

	static void tick() {
		m_deltaTime = m_clock.restart().asSeconds();
	}

private:
	static float m_deltaTime;
	static sf::Clock m_clock;
};