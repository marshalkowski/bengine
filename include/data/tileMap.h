#pragma once

#include <string>
#include <SFML/Graphics.hpp>

#include "../util/textureManager.h"

class TileMap : public sf::Drawable {

public:
	TileMap() {}

	TileMap(std::string textureFile, int cellSize, int tilesPerRow, std::vector<std::vector<int>> tiles) : 
		m_cellSize(cellSize),
		m_textureFile(textureFile), 
		m_tiles(tiles),
		m_textureRowSize(tilesPerRow) {
		initVertices();
	}

	void initVertices() {
		auto ySize = m_tiles.size();
		auto xSize = m_tiles[0].size();
		m_vertices = sf::VertexArray(sf::PrimitiveType::Triangles, 6 * xSize * ySize);
		for (auto y = 0; y < ySize; y++) {
			for (auto x = 0; x < xSize; x++) {
				addTile(x, y);
			}
		}
	}

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.texture = Global::textureManager().getTexture(m_textureFile);
		target.draw(m_vertices, states);
	}
	
	void addTile(float x, float y) {
		auto tIndex = m_tiles[y][x];
		m_vertices.append({ sf::Vector2f{ x, y }*m_cellSize, sf::Color::White, getTextureCoords(0.0f,0.0f, tIndex)});
		m_vertices.append({ sf::Vector2f{ x + 1.0f, y }*m_cellSize, sf::Color::White, getTextureCoords(1.0f,0.0f, tIndex) });
		m_vertices.append({ sf::Vector2f{ x + 1.0f, y + 1.0f }*m_cellSize, sf::Color::White, getTextureCoords(1.0f,1.0f, tIndex) });
		m_vertices.append({ sf::Vector2f{ x, y }*m_cellSize, sf::Color::White, getTextureCoords(0.0f,0.0f, tIndex) });
		m_vertices.append({ sf::Vector2f{ x + 1.0f, y + 1.0f }*m_cellSize, sf::Color::White, getTextureCoords(1.0f,1.0f, tIndex) });
		m_vertices.append({ sf::Vector2f{ x, y + 1.0f }*m_cellSize, sf::Color::White, getTextureCoords(0.0f,1.0f, tIndex) });
	}

	sf::Vector2f& getTextureCoords(float xPos, float yPos, int tileIndex) {
		auto col = tileIndex % m_textureRowSize;
		auto row = tileIndex / m_textureRowSize;
		return sf::Vector2((col + xPos) * m_cellSize, (row + yPos) * m_cellSize);
	}

public:
	int m_textureRowSize;
	float m_cellSize;
	std::string m_textureFile;
	std::vector<std::vector<int>> m_tiles;

private:
	sf::VertexArray m_vertices;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileMap, m_textureRowSize, m_cellSize, m_textureFile, m_tiles)