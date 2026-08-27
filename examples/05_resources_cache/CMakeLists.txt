add_executable(05_resources_cache
    main.cpp
)

target_link_libraries(05_resources_cache
    PRIVATE
        engine
)

# Same approach as the other sprite-loading examples: no asset-search-path
# concept in the engine yet, so the example bakes in its own asset directory
# at compile time.
target_compile_definitions(05_resources_cache
    PRIVATE
        RESOURCES_CACHE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
