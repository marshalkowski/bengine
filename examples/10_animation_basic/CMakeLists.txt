add_executable(10_animation_basic
    main.cpp
)

target_link_libraries(10_animation_basic
    PRIVATE
        bengine
)

# Same approach as the other asset-loading examples: no asset-search-path
# concept in the engine yet, so the example bakes in its own asset
# directory at compile time.
target_compile_definitions(10_animation_basic
    PRIVATE
        ANIMATION_BASIC_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
