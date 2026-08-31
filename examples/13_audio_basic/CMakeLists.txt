add_executable(13_audio_basic
    main.cpp
)

target_link_libraries(13_audio_basic
    PRIVATE
        engine
)

# Same approach as every other asset-loading example: no asset-search-path
# concept in the engine yet, so the example bakes in its own asset
# directory at compile time. beep.wav is a locally synthesized 250ms tone
# (see assets/), not a downloaded/external asset.
target_compile_definitions(13_audio_basic
    PRIVATE
        AUDIO_BASIC_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
