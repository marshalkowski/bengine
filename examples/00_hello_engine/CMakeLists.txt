add_executable(00_hello_engine
    main.cpp
)

# This example draws directly with raylib (no rendering abstraction exists
# yet), so it links raylib itself in addition to `engine`.
target_link_libraries(00_hello_engine
    PRIVATE
        engine
        raylib
)
