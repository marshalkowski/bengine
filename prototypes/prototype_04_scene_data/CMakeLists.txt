add_executable(prototype_04_scene_data
    main.cpp
)

target_link_libraries(prototype_04_scene_data
    PRIVATE
        engine
)

# Same approach as the other asset-loading prototypes: no asset-search-path
# concept in the engine yet, so the prototype bakes in its own asset
# directory at compile time. This tree also holds the external scene-data
# text files (assets/scenes/*.txt) -- there is no separate "data
# directory" concept, just more files under the same asset tree.
target_compile_definitions(prototype_04_scene_data
    PRIVATE
        PROTOTYPE_SCENE_DATA_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
