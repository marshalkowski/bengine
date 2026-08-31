add_library(bengine STATIC
    src/Engine.cpp
)

target_include_directories(bengine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# raylib is an implementation detail of bengine right now: consumers link
# against it only to get bengine's public API (namespace engine::), not
# raylib's.
target_link_libraries(bengine
    PRIVATE
        raylib
)

target_compile_features(bengine PUBLIC cxx_std_20)
