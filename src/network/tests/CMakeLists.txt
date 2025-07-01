# Enable testing
enable_testing()

add_executable(satox-network-tests
    network_manager_test.cpp
    network_vulnerability_tests.cpp
)

target_include_directories(satox-network-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

target_link_libraries(satox-network-tests
    PRIVATE
    satox-network
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)

# Add test to CTest
add_test(NAME satox-network-tests COMMAND satox-network-tests)

set_target_properties(satox-network-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../;$ORIGIN/../"
    BUILD_RPATH "$ORIGIN/../../;$ORIGIN/../"
)
