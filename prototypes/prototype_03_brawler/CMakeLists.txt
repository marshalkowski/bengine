add_executable(prototype_03_brawler
    main.cpp
)

target_link_libraries(prototype_03_brawler
    PRIVATE
        engine
)

# Same approach as the other asset-loading prototypes: no asset-search-path
# concept in the engine yet, so the prototype bakes in its own asset
# directory at compile time.
target_compile_definitions(prototype_03_brawler
    PRIVATE
        PROTOTYPE_BRAWLER_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
