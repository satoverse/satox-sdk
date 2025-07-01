# Enable testing
enable_testing()

add_executable(security_tests
    security_manager_test.cpp
    security_vulnerability_tests.cpp
)

target_include_directories(security_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

target_link_libraries(security_tests
    PRIVATE
    satox-security
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
    fmt::fmt
    spdlog::spdlog
)

# Add test to CTest
add_test(NAME security_tests COMMAND security_tests)

set_target_properties(security_tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../;$ORIGIN/../"
    BUILD_RPATH "$ORIGIN/../../;$ORIGIN/../"
)
