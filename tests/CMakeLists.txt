add_executable(
    tests
    test_order.cpp
    test_orderbook.cpp
)

target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/include
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# Only the `tests` target gets ORDERBOOK_ENABLE_TEST_HOOKS defined, so the
# friend struct in orderbook.h never appears in production (main) builds.
target_compile_definitions(tests PRIVATE ORDERBOOK_ENABLE_TEST_HOOKS)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(Catch)
catch_discover_tests(tests)

### "npm script"-style custom targets ###
# Usage: cmake --build <build-dir> --target app-test
add_custom_target(app-test
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS tests
    COMMENT "Running Catch2 test suite (ctest)"
    USES_TERMINAL
)

# Usage: cmake --build <build-dir> --target app-test-verbose
add_custom_target(app-test-verbose
    COMMAND tests --success
    WORKING_DIRECTORY $<TARGET_FILE_DIR:tests>
    DEPENDS tests
    COMMENT "Running Catch2 test suite directly (verbose)"
    USES_TERMINAL
)