cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
#fetch SFML
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

#fetch IMGUI
FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(ImGui)

FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)
set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml
    GIT_TAG v3.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(ImGui-SFML)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(main src/main.cpp "include/input.h" "include/chrono.h" "src/chrono.cpp"  "include/global.h" "include/ecs/entity.h" "include/ecs/components/transform.h" "include/util/vec2.h"  "include/ecs/entityManager.h" "include/ecs/componentArray.h" "include/ecs/componentManager.h" "include/ecs/system.h" "include/ecs/systemManager.h" "include/ecs/ecs.h" "include/ecs/systems/spriteRenderSystem.h" "include/ecs/components/spriteRenderer.h" "include/ecs/components/movable.h" "include/ecs/systems/movementSystem.h" "include/ecs/components/inputMovement.h" "include/ecs/systems/inputMoveSystem.h" "include/ecs/components/followMovement.h" "include/ecs/systems/followMoveSystem.h" "include/ecs/entityBuilder.h" "include/util/textureManager.h" "include/ecs/components/collider.h" "include/ecs/systems/collisionSystem.h"  "include/scene/demoScene.h" "include/ecs/components/tileMapRenderer.h" "include/ecs/systems/tileMapRenderSystem.h" "include/data/tileMap.h" "include/events/eventBus.h" "include/events/collisionEvent.h" "include/events/triggerEvents.h" "include/ecs/components/animatedSpriteRenderer.h" "include/ecs/systems/animatedSpriteRenderSystem.h")
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics ImGui-SFML::ImGui-SFML)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
