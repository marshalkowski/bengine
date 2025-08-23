#pragma once

#include <fstream>

#include "scene.h"

#include "..\ecs\components\sceneObject.h"
#include "..\ecs\ecs.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class SceneSaveLoad {

public:
	void Save(const Scene& scene, const std::string fileName) {
		json sceneJson;

		for (const auto e : scene.entities) {
			sceneJson["entities"].push_back(SerializeEntity(e));
		}

		std::ofstream file("assets\\scenes\\" + fileName + ".json");
		file << sceneJson.dump(4);
	}

	void Load(const std::string fileName) {
		std::ifstream file("assets\\scenes\\" + fileName + ".json");
		json sceneJson = json::parse(file);

		SceneManager::Init();

		for (const auto e : sceneJson["entities"]) {
			SceneManager::AddObject(DeserializeEntity(e));
		}

		ECSSystems::Cameras()->init(Global::config());
	}

	void LoadFromLevel(const std::string fileName) {
		std::ifstream file("assets\\levels\\" + fileName + ".json");
		json levelJson = json::parse(file);

		SceneManager::Init();

		auto map = levelJson["map"];

		auto tiles = std::vector<std::vector<int>>();
		for (auto y = 0; y < map.size(); y++) {
			tiles.push_back(std::vector<int>());
			for (auto x = 0; x < map[y].size(); x++) {
				tiles[y].push_back(map[y][x]);
				if (map[y][x] == 1) {
					SceneManager::AddObject(
						EntityBuilder()
						.CreateEntity("Map Collider " + std::to_string(x) + "," + std::to_string(y))
						.AddTransform(vec2(x * 32 + 16, y * 32 + 16))
						.AddCollider({ 32,32 })
						.ID()
					);
				}
			}
		}

		auto tileMap = CreateTileMap(TileMap(
			"assets\\sprites\\tile_crate.png",
			32,
			4,
			tiles
		));

		SceneManager::AddObject(tileMap);

		auto camera = EntityBuilder()
			.CreateEntity("Camera")
			.AddTransform(vec2(160.f, 160.f))
			.AddCamera()
			.ID();

		SceneManager::AddObject(camera);

		auto player = EntityBuilder()
			.CreateEntity("Player")
			.AddTransform(vec2(48, 48))
			.AddSprite("assets\\sprites\\player.png")
			.AddMovable(Global::config()["player"]["speed"])
			.AddInputMovement()
			.AddCollider({ 22,26 })
			.ID();

		SceneManager::AddObject(player);

		auto crate = EntityBuilder()
			.CreateEntity("Crate")
			.AddTransform(vec2(176, 176))
			.AddSprite("assets\\sprites\\crate.png")
			.AddCollider({ 32,32 })
			.AddDestructible()
			.ID();

		SceneManager::AddObject(crate);
			

		ECSSystems::Cameras()->init(Global::config());
	}

private:
	json SerializeEntity(Entity e) {
		json entityJson;

		SceneObject sceneObj = ECS::GetComponent<SceneObject>(e);

		entityJson["name"] = sceneObj.name;

		if (ECS::HasComponent<AnimatedSpriteRenderer>(e)) {
			entityJson["animatedSpriteRenderer"] = ECS::GetComponent<AnimatedSpriteRenderer>(e);
		}
		if (ECS::HasComponent<Camera>(e)) {
			entityJson["camera"] = ECS::GetComponent<Camera>(e);
		}
		if (ECS::HasComponent<Collider>(e)) {
			entityJson["collider"] = ECS::GetComponent<Collider>(e);
		}
		if (ECS::HasComponent<FollowMovement>(e)) {
			json fm;
			fm["name"] = ECS::GetComponent<SceneObject>(ECS::GetComponent<FollowMovement>(e).target).name;
			entityJson["followMovement"] = fm;
		}
		if (ECS::HasComponent<InputMovement>(e)) {
			entityJson["inputMovement"] = {};
		}
		if (ECS::HasComponent<Movable>(e)) {
			entityJson["movable"] = ECS::GetComponent<Movable>(e);
		}
		if (ECS::HasComponent<SpriteRenderer>(e)) {
			entityJson["spriteRenderer"] = ECS::GetComponent<SpriteRenderer>(e);
		}
		if (ECS::HasComponent<TileMapRenderer>(e)) {
			entityJson["tileMapRenderer"] = ECS::GetComponent<TileMapRenderer>(e);
		}
		if (ECS::HasComponent<Transform>(e)) {
			entityJson["transform"] = ECS::GetComponent<Transform>(e);
		}

		if (sceneObj.children.size() > 0) {
			for (auto child : sceneObj.children) {
				entityJson["children"].push_back(SerializeEntity(child));
			}
		}

		return entityJson;
	}

	Entity DeserializeEntity(json e) {
		auto eb = EntityBuilder().CreateEntity(e["name"]);
		if (e.contains("animatedSpriteRenderer")) {
			eb.AddAnimatedSprite(e["animatedSpriteRenderer"]["frames"]);
		}
		if (e.contains("camera")) {
			eb.AddCamera();
		}
		if (e.contains("collider")) {
			eb.AddCollider(e["collider"]["size"], e["collider"]["trigger"]);
		}
		if (e.contains("followMovement")) {
			eb.AddFollowMovement(SceneManager::GetObjectByName(e["followMovement"]["name"]));
		}
		if (e.contains("inputMovement")) {
			eb.AddInputMovement();
		}
		if (e.contains("movable")) {
			eb.AddMovable(e["movable"]["speed"]);
		}
		if (e.contains("spriteRenderer")) {
			eb.AddSprite(e["spriteRenderer"]["path"]);
		}
		if (e.contains("tileMapRenderer")) {
			TileMap map = e["tileMapRenderer"]["tileMap"];
			map.initVertices();

			eb.AddTileMap(map);
		}
		if (e.contains("transform")) {
			eb.AddTransform(e["transform"]["position"]);
		}

		if (e.contains("children")) {
			auto parent = eb.ID();

			for (auto c : e["children"]) {
				auto child = DeserializeEntity(c);
				SceneManager::ParentChild(parent, child);
			}
		}

		return eb.ID();
	}

};