cmake_minimum_required(VERSION 3.24)

project(bengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# raylib, fetched at configure time so a fresh clone needs nothing pre-installed.
# ---------------------------------------------------------------------------
include(FetchContent)

# raylib's own CMake build produces examples/games we don't want built.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_subdirectory(engine)

# examples/: narrow, one-capability-at-a-time demonstrations of the engine API.
add_subdirectory(examples/00_hello_engine)
add_subdirectory(examples/01_hello_sprite)
add_subdirectory(examples/02_time)
add_subdirectory(examples/03_input)
add_subdirectory(examples/04_resources_basic)
add_subdirectory(examples/05_resources_cache)
add_subdirectory(examples/06_input_states)
add_subdirectory(examples/07_primitives)
add_subdirectory(examples/08_geometry)
add_subdirectory(examples/09_float_rendering)
add_subdirectory(examples/10_animation_basic)
add_subdirectory(examples/11_animation_clip)

# prototypes/: small games combining existing capabilities, used to surface
# what the engine needs next. Each is its own executable, just like a real game.
add_subdirectory(prototypes/prototype_00_topdown)
add_subdirectory(prototypes/prototype_01_continuous)
add_subdirectory(prototypes/prototype_02_projectiles)
add_subdirectory(prototypes/prototype_03_brawler)
add_subdirectory(prototypes/prototype_04_scene_data)
add_subdirectory(prototypes/prototype_05_board_game)
