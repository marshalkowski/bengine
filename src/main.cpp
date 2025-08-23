#include <SFML/Graphics.hpp>
#include <fstream>
//s#include <random>
#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

#include "../include/input.h"
#include "../include/chrono.h"
#include "../include/global.h"
#include "../include/editor/editorInterface.h"
#include "../include/scene/demoScene.h"
#include "../include/scene/sceneManager.h"
#include "../include/scene/sceneSaveLoad.h"
#include "../include/util/vec2.h"
#include "../include/util/textureManager.h"

#include "../include/ecs/ecs.h"
#include "../include/ecs/ecsSystems.h"
#include "../include/ecs/entityBuilder.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main()
{
    SceneManager sceneManager;
    EditorInterface editorInterface;

    std::ifstream configFile("assets\\config.json");
    auto c = json::parse(configFile);
    Global::setConfig(c);
    Global::setDebug(c["debug"]);

    auto config = Global::config();
    sf::RenderWindow window(sf::VideoMode({ config["windowSize"]["width"], config["windowSize"]["height"] }), "Bengine 0.1.0a", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(144);

    ImGui::SFML::Init(window);
    sf::Clock clock;

    auto view = sf::View(sf::FloatRect({ 0.0f,0.0f }, { (float)config["windowSize"]["width"], (float)config["windowSize"]["height"] }));
    window.setView(view);

    Chrono::init();

    ECSSystems::Init();

    if (!config["debug"]) {
        sceneManager.LoadDemoScene(config);
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
        ECSSystems::InputMove()->update();
        ECSSystems::FollowMove()->update();

        // physics
        ECSSystems::Collision()->update(Chrono::deltaTime());

        // game logic
        ECSSystems::Movement()->update(Chrono::deltaTime());

        // render
        ECSSystems::Cameras()->update(window);
        window.clear();
        ECSSystems::TileMapRender()->update(window, Global::textureManager());
        ECSSystems::SpriteRender()->update(window, Global::textureManager());
        ECSSystems::AnimatedSpriteRender()->update(window, Global::textureManager(), Chrono::deltaTime());
        if (config["debug"]) {
            ECSSystems::SpriteRender()->debug(window, Global::textureManager());
            ECSSystems::Collision()->debug(window);
            editorInterface.Render();
            /*ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Once);
            ImGui::SetNextWindowSize({ 400, 150 }, ImGuiCond_Once);
            ImGui::Begin("Bengine v.0.1");
            if (ImGui::Button("Load Scene")) {
                SceneManager::LoadDemoScene(config);
            }
            if (ImGui::Button("Load From File")) {
                SceneSaveLoad load;
                load.Load("testScene");
            }
            if (ImGui::Button("Spawn Box")) {
                DemoScene::SpawnBox(config);
            }
            if (ImGui::Button("Clear Scene")) {
                sceneManager.ClearScene();
            }
            bool sceneLoaded = sceneManager.IsSceneLoaded();
            if (!sceneLoaded) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            }
            if (ImGui::Button("Save Scene")) {
                SceneSaveLoad save;
                save.Save(sceneManager.GetScene(), "testScene");
            }
            if (!sceneLoaded) {
                ImGui::PopItemFlag();
            }
            ImGui::End();
            ImGui::SetNextWindowPos({ 0, 150 }, ImGuiCond_Once);
            ImGui::SetNextWindowSize({ 400, 100 }, ImGuiCond_Once);
            ImGui::Begin("Performance");
            const auto frameRate = std::to_string(1.0f / Chrono::deltaTime());
            ImGui::Text((std::string("Frame Rate ") + frameRate).c_str());
            ImGui::End();
            ImGui::SetNextWindowPos({ 0, 250 }, ImGuiCond_Once);
            ImGui::SetNextWindowSize({ 400, 800 }, ImGuiCond_Once);
            ImGui::Begin("Scene");
            if (SceneManager::IsSceneLoaded()) {
                for (auto e : SceneManager::GetScene().entities) {
                    ImGui::Selectable(ECS::GetComponent<SceneObject>(e).name.c_str());
                }
            }
            ImGui::End();*/

        }
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}
