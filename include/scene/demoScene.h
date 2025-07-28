#pragma once

#include <random>

#include "../ecs/entity.h"
#include "../ecs/entityBuilder.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void CreateOrb(float x, float y, Entity target, float speed) {
    EntityBuilder()
        .CreateEntity()
        .AddTransform(vec2(x, y))
        .AddSprite("assets\\sprites\\orb.png")
        .AddMovable(speed)
        .AddFollowMovement(target)
        .AddCollider({ 20,20 });
}

void CreateBox(float x, float y) {
    EntityBuilder()
        .CreateEntity()
        .AddTransform(vec2(x, y))
        .AddSprite("assets\\sprites\\box.png")
        .AddCollider({ 32,32 });
}

void CreateTileMap(TileMap tileMap) {
    EntityBuilder()
        .CreateEntity()
        .AddTileMap(tileMap);
}

class DemoScene {
public:
	static void Create(json config) {
        auto entity = EntityBuilder()
            .CreateEntity()
            .AddTransform(vec2(config["player"]["position"]["x"], config["player"]["position"]["y"]))
            .AddSprite("assets\\sprites\\player.png")
            .AddMovable(config["player"]["speed"])
            .AddInputMovement()
            .AddCollider({ 22,26 })
            .ID();

        EntityBuilder()
            .CreateEntity()
            .AddTransform(vec2(config["player"]["position"]["x"] + 100.0f, config["player"]["position"]["y"]))
            .AddAnimatedSprite({ 
                "assets\\sprites\\player.png", 
                "assets\\sprites\\player_002.png",
                "assets\\sprites\\player_003.png",
                "assets\\sprites\\player_002.png"
            })
            .AddMovable(config["player"]["speed"])
            .ID();


        auto tiles = std::vector<std::vector<int>>();
        for (auto y = 0; y < 34; y++) {
            tiles.push_back(std::vector<int>());
            for (auto x = 0; x < 60; x++) {
                tiles[y].push_back(1);
            }
        }

        CreateTileMap(TileMap(
            "assets\\sprites\\tile_texture.png",
            32,
            4,
            tiles
        ));

        EntityBuilder()
            .CreateEntity()
            .AddTransform({ 1000,1000 })
            .AddCollider({ 32,32 }, true);

        std::random_device rd;  // a seed source for the random number engine
        std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> rand_x(1, config["windowSize"]["width"]);
        std::uniform_int_distribution<> rand_y(1, config["windowSize"]["height"]);

        for (int i = 0; i < config["orb"]["count"]; i++) {
            CreateOrb(rand_x(gen), rand_y(gen), entity, config["orb"]["speed"]);
        }

        CreateBox(500, 500);
        CreateBox(500, 532);
        CreateBox(500, 564);

	}
};