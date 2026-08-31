add_executable(03_input
    main.cpp
)

target_link_libraries(03_input
    PRIVATE
        bengine
)

# Same approach as 01_hello_sprite: no asset-loading concept in the engine
# yet, so the example bakes in its own asset directory at compile time.
target_compile_definitions(03_input
    PRIVATE
        INPUT_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
