add_executable(14_camera_basic
    main.cpp
)

target_link_libraries(14_camera_basic
    PRIVATE
        bengine
)

# Bengine consumer asset workflow: copy this example's own assets/ next to
# the built executable (POST_BUILD, so it re-copies whenever the target is
# rebuilt), rather than baking a compile-time source-tree path into a
# compile definition like the other examples do. $<TARGET_FILE_DIR:...> is
# this target's own output directory for the active config, so for the
# multi-config Visual Studio generator this naturally lands at
# build/examples/14_camera_basic/Debug/assets (Release/assets, etc.) --
# matching what Engine::SetAssetRoot("assets") resolves against at runtime
# (see main.cpp). Plain CMake; nothing Bengine- or game-specific here, so
# any consumer can copy this pattern as-is for its own assets/.
add_custom_command(TARGET 14_camera_basic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:14_camera_basic>/assets"
)
