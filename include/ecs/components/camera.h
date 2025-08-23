#pragma once

#include <SFML/Graphics.hpp>

struct Camera {
	sf::View view;
	float currentZoom;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera, currentZoom)