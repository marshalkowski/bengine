#pragma once

#include <SFML/Graphics.hpp>

#include "../system.h"
#include "../ecs.h"
#include "../components/animatedSpriteRenderer.h"
#include "../components/transform.h"
#include "../../util/textureManager.h"

class AnimatedSpriteRenderSystem : public System {
public:
	void update(sf::RenderWindow& window, TextureManager& textureManager, float dt) {
		int advanceByFrames = 0;
		m_accumulator += dt;
		while (m_accumulator > m_timeStep) {
			advanceByFrames += 1;
			m_accumulator -= m_timeStep;
		}

		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& spriteRenderer = ECS::GetComponent<AnimatedSpriteRenderer>(entity);
			if (advanceByFrames) {
				spriteRenderer.frameIndex = (spriteRenderer.frameIndex + 1) % spriteRenderer.frames.size();
			}
			auto texture = *textureManager.getTexture(spriteRenderer.frames[spriteRenderer.frameIndex]);

			sf::Sprite sprite(texture);
			sprite.setPosition(sf::Vector2f(transform.position.x - texture.getSize().x / 2, transform.position.y - texture.getSize().y / 2));

			window.draw(sprite);
		}
	}

	void debug(sf::RenderWindow& window, TextureManager& textureManager) {
		for (auto const& entity : m_entities) {
			auto& transform = ECS::GetComponent<Transform>(entity);
			auto& spriteRenderer = ECS::GetComponent<AnimatedSpriteRenderer>(entity);

			auto const texture = textureManager.getTexture(spriteRenderer.frames[spriteRenderer.frameIndex]);
			auto const size = texture->getSize();

			sf::RectangleShape rect({ (float)size.x,(float)size.y });
			rect.setPosition({ transform.position.x - texture->getSize().x / 2, transform.position.y - texture->getSize().y / 2 });
			rect.setOutlineColor(sf::Color::Cyan);
			rect.setOutlineThickness(1.0f);
			rect.setFillColor(sf::Color::Transparent);
			window.draw(rect);

		}
	}

private:
	const float m_timeStep = 1.0f / 24.0f;
	float m_accumulator = 0.0f;
};