#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <iostream>

class TextureManager {
public:
	sf::Texture* getTexture(std::string filePath) {
		const auto result = m_textures.find(filePath);
		if (result == m_textures.end()) {
			sf::Texture tex = sf::Texture(filePath);

			m_textures[filePath] = std::make_unique<sf::Texture>(tex);
		}
		return m_textures[filePath].get();
	};

private:
	std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_textures;
};