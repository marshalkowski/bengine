#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

#include "../chrono.h"
#include "../global.h"
#include "../scene/sceneManager.h"
#include "../scene/sceneSaveLoad.h"


class EditorInterface {
public:
	void Render() {
        ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ 400, 150 }, ImGuiCond_Once);
        ImGui::Begin("Bengine v.0.1");
        if (ImGui::Button("Load Scene")) {
            SceneManager::LoadDemoScene(Global::config());
        }
        if (ImGui::Button("Load From File")) {
            SceneSaveLoad load;
            load.Load("testScene");
        }
        if (ImGui::Button("Load From Level")) {
            SceneSaveLoad load;
            load.LoadFromLevel("1");
        }
        if (ImGui::Button("Spawn Box")) {
            DemoScene::SpawnBox(Global::config());
        }
        if (ImGui::Button("Clear Scene")) {
            SceneManager::ClearScene();
        }
        bool sceneLoaded = SceneManager::IsSceneLoaded();
        if (!sceneLoaded) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }
        if (ImGui::Button("Save Scene")) {
            SceneSaveLoad save;
            save.Save(SceneManager::GetScene(), "testSceneSmallMap");
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
                if (ImGui::Selectable((ECS::GetComponent<SceneObject>(e).name).c_str())) {
                    m_selected = e;
                }
            }
        }
        else {
            m_selected = -1;
        }
        ImGui::End();
        ImGui::SetNextWindowPos({ 1920-400,0 }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ 400, 800 }, ImGuiCond_Once);
        ImGui::Begin("Inspector");
        if (SceneManager::IsSceneLoaded() && m_selected != -1) {
            ImGui::Text(ECS::GetComponent<SceneObject>(m_selected).name.c_str());
        }
        ImGui::End();
	}

private:
    unsigned int m_selected = -1;

};