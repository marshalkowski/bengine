add_executable(11_animation_clip
    main.cpp
)

target_link_libraries(11_animation_clip
    PRIVATE
        bengine
)

# Same approach as the other asset-loading examples: no asset-search-path
# concept in the engine yet, so the example bakes in its own asset
# directory at compile time.
target_compile_definitions(11_animation_clip
    PRIVATE
        ANIMATION_CLIP_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
