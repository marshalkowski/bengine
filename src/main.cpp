#include <SFML/Graphics.hpp>
#include <fstream>
#include <random>

#include <imgui.h>
#include <imgui-SFML.h>

#include "../include/input.h"
#include "../include/chrono.h"
#include "../include/global.h"
#include "../include/scene/demoScene.h"
#include "../include/util/vec2.h"
#include "../include/util/textureManager.h"

#include "../include/ecs/ecs.h"
#include "../include/ecs/entityBuilder.h"
#include "../include/ecs/components/animatedSpriteRenderer.h"
#include "../include/ecs/components/collider.h"
#include "../include/ecs/components/transform.h"
#include "../include/ecs/components/spriteRenderer.h"
#include "../include/ecs/components/movable.h"
#include "../include/ecs/components/inputMovement.h"
#include "../include/ecs/components/followMovement.h"
#include "../include/ecs/components/tileMapRenderer.h"

#include "../include/ecs/systems/animatedSpriteRenderSystem.h"
#include "../include/ecs/systems/collisionSystem.h"
#include "../include/ecs/systems/spriteRenderSystem.h"
#include "../include/ecs/systems/movementSystem.h"
#include "../include/ecs/systems/inputMoveSystem.h"
#include "../include/ecs/systems/followMoveSystem.h"
#include "../include/ecs/systems/tileMapRenderSystem.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::shared_ptr<SpriteRenderSystem> spriteRenderer;
std::shared_ptr<MovementSystem> movementSystem;
std::shared_ptr<InputMoveSystem> inputMoveSystem;
std::shared_ptr<FollowMoveSystem> followMoveSystem;
std::shared_ptr<CollisionSystem> collisionSystem;
std::shared_ptr<TileMapRenderSystem> tileMapRenderer;
std::shared_ptr<AnimatedSpriteRenderSystem> animatedSpriteRenderer;

void InitECS() {
    ECS::RegisterComponent<Transform>();
    ECS::RegisterComponent<SpriteRenderer>();
    ECS::RegisterComponent<Movable>();
    ECS::RegisterComponent<InputMovement>();
    ECS::RegisterComponent<FollowMovement>();
    ECS::RegisterComponent<Collider>();
    ECS::RegisterComponent<TileMapRenderer>();
    ECS::RegisterComponent<AnimatedSpriteRenderer>();

    spriteRenderer = ECS::RegisterSystem<SpriteRenderSystem>();
    Signature spriteSignature;
    spriteSignature.set(ECS::GetComponentType<Transform>());
    spriteSignature.set(ECS::GetComponentType<SpriteRenderer>());
    ECS::SetSystemSignature<SpriteRenderSystem>(spriteSignature);

    movementSystem = ECS::RegisterSystem<MovementSystem>();
    Signature movementSignature;
    movementSignature.set(ECS::GetComponentType<Transform>());
    movementSignature.set(ECS::GetComponentType<Movable>());
    ECS::SetSystemSignature<MovementSystem>(movementSignature);
    movementSystem->init();

    inputMoveSystem = ECS::RegisterSystem<InputMoveSystem>();
    Signature inputMoveSignature;
    inputMoveSignature.set(ECS::GetComponentType<InputMovement>());
    inputMoveSignature.set(ECS::GetComponentType<Movable>());
    ECS::SetSystemSignature<InputMoveSystem>(inputMoveSignature);

    followMoveSystem = ECS::RegisterSystem<FollowMoveSystem>();
    Signature followMoveSignature;
    followMoveSignature.set(ECS::GetComponentType<FollowMovement>());
    followMoveSignature.set(ECS::GetComponentType<Movable>());
    followMoveSignature.set(ECS::GetComponentType<Transform>());
    ECS::SetSystemSignature<FollowMoveSystem>(followMoveSignature);

    collisionSystem = ECS::RegisterSystem<CollisionSystem>();
    Signature collisionSignature;
    collisionSignature.set(ECS::GetComponentType<Transform>());
    collisionSignature.set(ECS::GetComponentType<Collider>());
    ECS::SetSystemSignature<CollisionSystem>(collisionSignature);

    tileMapRenderer = ECS::RegisterSystem<TileMapRenderSystem>();
    Signature tileMapSignature;
    tileMapSignature.set(ECS::GetComponentType<TileMapRenderer>());
    ECS::SetSystemSignature<TileMapRenderSystem>(tileMapSignature);

    animatedSpriteRenderer = ECS::RegisterSystem<AnimatedSpriteRenderSystem>();
    Signature animSpriteSignature;
    animSpriteSignature.set(ECS::GetComponentType<AnimatedSpriteRenderer>());
    animSpriteSignature.set(ECS::GetComponentType<Transform>());
    ECS::SetSystemSignature<AnimatedSpriteRenderSystem>(animSpriteSignature);
}

int main()
{
    std::ifstream configFile("assets\\config.json");
    const auto config = json::parse(configFile);
    Global::setDebug(config["debug"]);

    sf::RenderWindow window(sf::VideoMode({ config["windowSize"]["width"], config["windowSize"]["height"] }), "Bengine 0.1.0a", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(144);

    ImGui::SFML::Init(window);
    sf::Clock clock;

    Chrono::init();

    sf::Text text(Global::font(), config["initMsg"].get<std::string>(), 20U);

    InitECS();

    if (!config["debug"]) {
        DemoScene::Create(config);
    }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event.has_value()) {
                ImGui::SFML::ProcessEvent(window, event.value());
            }
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        Chrono::tick();

        ImGui::SFML::Update(window, clock.restart());

        // input?
        inputMoveSystem->update();
        followMoveSystem->update();

        // physics
        collisionSystem->update(Chrono::deltaTime());

        // game logic
        movementSystem->update(Chrono::deltaTime());

        text.setString(std::to_string(1.0f / Chrono::deltaTime()));

        // render
        window.clear();
        tileMapRenderer->update(window, Global::textureManager());
        window.draw(text);
        spriteRenderer->update(window, Global::textureManager());
        animatedSpriteRenderer->update(window, Global::textureManager(), Chrono::deltaTime());
        if (config["debug"]) {
            spriteRenderer->debug(window, Global::textureManager());
            collisionSystem->debug(window);
            // ImGui::ShowDemoWindow();
            ImGui::SetNextWindowPos({ 100, 100 }, ImGuiCond_Once);
            ImGui::SetNextWindowSize({ 200, 150 }, ImGuiCond_Once);
            ImGui::Begin("Bengine v.0.1");
            if (ImGui::Button("Load Scene")) {
                DemoScene::Create(config);
            }
            if (ImGui::Button("Spawn Box")) {
                DemoScene::SpawnBox(config);
            }
            ImGui::End();
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}
