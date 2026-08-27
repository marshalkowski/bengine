add_executable(04_resources_basic
    main.cpp
)

target_link_libraries(04_resources_basic
    PRIVATE
        engine
)

# Same approach as 01_hello_sprite/03_input: no asset-search-path concept in
# the engine yet, so the example bakes in its own asset directory at compile time.
target_compile_definitions(04_resources_basic
    PRIVATE
        RESOURCES_BASIC_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
