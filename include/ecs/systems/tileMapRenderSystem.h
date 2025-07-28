#pragma once

#include <memory>
#include <iostream>

#include "../system.h"

class TileMapRenderSystem : public System {
public:
	void update(sf::RenderWindow& window, TextureManager& textureManager) {
		for (auto const& entity : m_entities) {
			auto& tileMap = ECS::GetComponent<TileMapRenderer>(entity);
			window.draw(tileMap.tileMap);
		}
	}

private:


};