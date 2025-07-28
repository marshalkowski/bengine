#pragma once

#include <SFML/Graphics.hpp>

#include "../system.h"
#include "../ecs.h"
#include "../../util/textureManager.h"

class SpriteRenderSystem : public System {
public:
	void update(sf::RenderWindow& window, TextureManager& textureManager) {
		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& spriteRenderer = ECS::GetComponent<SpriteRenderer>(entity);
			auto texture = *textureManager.getTexture(spriteRenderer.path);

			sf::Sprite sprite(texture);
			sprite.setPosition(sf::Vector2f(transform.position.x - texture.getSize().x / 2, transform.position.y - texture.getSize().y / 2));

			window.draw(sprite);
		}
	}

	void debug(sf::RenderWindow& window, TextureManager& textureManager) {
		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& spriteRenderer = ECS::GetComponent<SpriteRenderer>(entity);

			auto const texture = textureManager.getTexture(spriteRenderer.path);
			auto const size = texture->getSize();

			sf::RectangleShape rect({(float)size.x,(float)size.y});
			rect.setPosition({ transform.position.x - texture->getSize().x / 2, transform.position.y - texture->getSize().y / 2 });
			rect.setOutlineColor(sf::Color::Cyan);
			rect.setOutlineThickness(1.0f);
			rect.setFillColor(sf::Color::Transparent);
			window.draw(rect);

		}
	}
};