#pragma once

#include <SFML/Window.hpp>

class Input {
public:
	static bool space() {
		return getKey(sf::Keyboard::Key::Space);
	}
	static bool up() {
		return getKey(sf::Keyboard::Key::Up);
	}
	static bool down() {
		return getKey(sf::Keyboard::Key::Down);
	}
	static bool left() {
		return getKey(sf::Keyboard::Key::Left);
	}
	static bool right() {
		return getKey(sf::Keyboard::Key::Right);
	}


private:
	static bool getKey(sf::Keyboard::Key key) {
		return sf::Keyboard::isKeyPressed(key);
	}


};