#pragma once

#include <SFML\Graphics.hpp>

#include "util/textureManager.h"
#include "events/eventBus.h"

class Global {
public:
	static Global& Instance() {
		static Global instance;
		return instance;
	}

	static const sf::Font& font() {
		return Instance().m_font;
	}

	static TextureManager& textureManager() {
		return *Instance().m_textureManager.get();
	}

	static EventBus& eventBus() {
		return Instance().m_eventBus;
	}

	static void setDebug(bool d) {
		Instance().m_debug = d;
	}

	static bool isDebug() {
		return Instance().m_debug;
	}

private:
	Global() {
		m_font = sf::Font("assets\\fonts\\arial.ttf");
		m_textureManager = std::make_shared<TextureManager>();
		m_eventBus = EventBus();
	}

	sf::Font m_font;
	std::shared_ptr<TextureManager> m_textureManager;
	EventBus m_eventBus;
	bool m_debug;
};