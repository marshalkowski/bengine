add_executable(prototype_00_topdown
    main.cpp
)

target_link_libraries(prototype_00_topdown
    PRIVATE
        bengine
)

# Same approach as the sprite-loading examples: no asset-search-path concept
# in the engine yet, so the prototype bakes in its own asset directory at
# compile time.
target_compile_definitions(prototype_00_topdown
    PRIVATE
        PROTOTYPE_TOPDOWN_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
