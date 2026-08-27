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
add_subdirectory(examples/00_hello_engine)
add_subdirectory(examples/01_hello_sprite)
add_subdirectory(examples/02_time)
add_subdirectory(examples/03_input)
add_subdirectory(examples/04_resources_basic)
add_subdirectory(examples/05_resources_cache)
