#pragma once

#include "..\..\data\tileMap.h"

struct TileMapRenderer {
	TileMap tileMap;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileMapRenderer, tileMap)