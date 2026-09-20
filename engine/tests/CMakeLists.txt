add_executable(bengine_tests
    EngineTests.cpp
)

target_link_libraries(bengine_tests
    PRIVATE
        bengine
)

add_test(NAME bengine_tests COMMAND bengine_tests)
