add_executable(prototype_01_continuous
    main.cpp
)

target_link_libraries(prototype_01_continuous
    PRIVATE
        engine
)

# Same approach as the other asset-loading examples/prototypes: no
# asset-search-path concept in the engine yet, so the prototype bakes in its
# own asset directory at compile time.
target_compile_definitions(prototype_01_continuous
    PRIVATE
        PROTOTYPE_CONTINUOUS_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
