# Enable testing
enable_testing()

add_executable(ipfs_tests
    ipfs_manager_test.cpp
)

target_include_directories(ipfs_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

target_link_libraries(ipfs_tests
    PRIVATE
    satox-ipfs
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)

# Add test to CTest
add_test(NAME ipfs_tests COMMAND ipfs_tests)

set_target_properties(ipfs_tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
