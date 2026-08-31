cmake_minimum_required(VERSION 3.24)

project(Bengine LANGUAGES CXX)

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

# ---------------------------------------------------------------------------
# examples/ and prototypes/ are Bengine's own development workflow -- API
# documentation and architectural pressure tests, not something a project
# that consumes Bengine via add_subdirectory() should build. Default ON when
# Bengine is being built as its own top-level project (normal Bengine
# development) and OFF when it's being pulled in as another project's
# dependency (e.g. a git submodule under vendor/), using CMake's own
# top-level-project detection rather than a custom variable. A consumer that
# genuinely wants them can still turn either option back on explicitly.
# ---------------------------------------------------------------------------
option(BENGINE_BUILD_EXAMPLES "Build Bengine's examples" ${Bengine_IS_TOP_LEVEL})
option(BENGINE_BUILD_PROTOTYPES "Build Bengine's prototypes" ${Bengine_IS_TOP_LEVEL})

# examples/: narrow, one-capability-at-a-time demonstrations of the engine API.
if(BENGINE_BUILD_EXAMPLES)
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
    add_subdirectory(examples/12_pointer_input)
    add_subdirectory(examples/13_audio_basic)
endif()

# prototypes/: small games combining existing capabilities, used to surface
# what the engine needs next. Each is its own executable, just like a real game.
if(BENGINE_BUILD_PROTOTYPES)
    add_subdirectory(prototypes/prototype_00_topdown)
    add_subdirectory(prototypes/prototype_01_continuous)
    add_subdirectory(prototypes/prototype_02_projectiles)
    add_subdirectory(prototypes/prototype_03_brawler)
    add_subdirectory(prototypes/prototype_04_scene_data)
    add_subdirectory(prototypes/prototype_05_board_game)
endif()
