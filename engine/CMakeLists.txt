add_library(engine STATIC
    src/Engine.cpp
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# raylib is an implementation detail of the engine right now: consumers of
# `engine` link against it only to get engine's public API, not raylib's.
target_link_libraries(engine
    PRIVATE
        raylib
)

target_compile_features(engine PUBLIC cxx_std_20)
