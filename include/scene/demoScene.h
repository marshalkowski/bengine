#pragma once

#include <random>

#include "../ecs/entity.h"
#include "../ecs/entityBuilder.h"

#include "scene.h"
#include "sceneUtils.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

Entity CreateOrb(float x, float y, Entity target, float speed) {
    return EntityBuilder()
        .CreateEntity("Orb")
        .AddTransform(vec2(x, y))
        .AddSprite("assets\\sprites\\orb.png")
        .AddMovable(speed)
        .AddFollowMovement(target)
        .AddCollider({ 20,20 })
        .ID();
}

Entity CreateBox(float x, float y) {
    return EntityBuilder()
        .CreateEntity("Box")
        .AddTransform(vec2(x, y))
        .AddSprite("assets\\sprites\\box.png")
        .AddCollider({ 32,32 })
        .ID();
}

Entity CreateTileMap(TileMap tileMap) {
    return EntityBuilder()
        .CreateEntity("Tile Map")
        .AddTileMap(tileMap)
        .ID();
}

class DemoScene {
public:
	static Scene Create(json config) {
        Scene scene;

        auto player = EntityBuilder()
            .CreateEntity("Player")
            .AddTransform(vec2(config["player"]["position"]["x"], config["player"]["position"]["y"]))
            .AddSprite("assets\\sprites\\player.png")
            .AddMovable(config["player"]["speed"])
            .AddInputMovement()
            .AddCollider({ 22,26 })
            .ID();

        SceneUtils::AddEntityToScene(player, scene);

        // animated sprite
        SceneUtils::AddEntityToScene(EntityBuilder()
            .CreateEntity("Animation Demo")
            .AddTransform(vec2(config["player"]["position"]["x"] + 100.0f, config["player"]["position"]["y"]))
            .AddAnimatedSprite({ 
                "assets\\sprites\\player.png", 
                "assets\\sprites\\player_002.png",
                "assets\\sprites\\player_003.png",
                "assets\\sprites\\player_002.png"
            })
            .ID(), scene);

        auto camera = EntityBuilder()
            .CreateEntity("Camera")
            .AddTransform(vec2(config["player"]["position"]["x"], config["player"]["position"]["y"]))
            .AddCamera()
            .ID();
        
        auto tiles = std::vector<std::vector<int>>();
        for (auto y = 0; y < 8; y++) { // 34
            tiles.push_back(std::vector<int>());
            for (auto x = 0; x < 8; x++) { // 60
                tiles[y].push_back(1);
            }
        }

        auto tileMap = CreateTileMap(TileMap(
            "assets\\sprites\\tile_texture.png",
            32,
            4,
            tiles
        ));

        SceneUtils::AddEntityToScene(tileMap, scene);

        // trigger collider
        SceneUtils::AddEntityToScene(EntityBuilder()
            .CreateEntity("Trigger Demo")
            .AddTransform({ 1000,1000 })
            .AddCollider({ 32,32 }, true)
            .ID(), scene);

        std::random_device rd;  // a seed source for the random number engine
        std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> rand_x(1, config["windowSize"]["width"]);
        std::uniform_int_distribution<> rand_y(1, config["windowSize"]["height"]);

        for (int i = 0; i < config["orb"]["count"]; i++) {
            SceneUtils::AddEntityToScene(CreateOrb(rand_x(gen), rand_y(gen), player, config["orb"]["speed"]), scene);
        }

        SceneUtils::AddEntityToScene(CreateBox(500, 532), scene);
        SceneUtils::AddEntityToScene(CreateBox(500, 564), scene);
        SceneUtils::AddEntityToScene(CreateBox(500, 500), scene);

        SceneUtils::ParentChild(player, camera, scene);

        return scene;
	}

    static void SpawnBox(json config) {
        std::random_device rd;  // a seed source for the random number engine
        std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> rand_x(1, config["windowSize"]["width"]);
        std::uniform_int_distribution<> rand_y(1, config["windowSize"]["height"]);

        CreateBox(rand_x(gen), rand_y(gen));
    }
};