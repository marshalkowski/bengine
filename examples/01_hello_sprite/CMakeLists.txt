add_executable(01_hello_sprite
    main.cpp
)

target_link_libraries(01_hello_sprite
    PRIVATE
        bengine
)

# The engine has no asset-loading/search-path concept yet (that's resource
# manager territory, deliberately not built here), so the example just bakes
# in the absolute path to its own asset directory at compile time.
target_compile_definitions(01_hello_sprite
    PRIVATE
        HELLO_SPRITE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
