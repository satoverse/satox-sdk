# Enable testing
enable_testing()

add_executable(satox-blockchain-tests
    blockchain_manager_test.cpp
    block_test.cpp
    blockchain_vulnerability_tests.cpp
)

target_include_directories(satox-blockchain-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

target_link_libraries(satox-blockchain-tests
    PRIVATE
    satox-blockchain
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)

# Add test to CTest
add_test(NAME satox-blockchain-tests COMMAND satox-blockchain-tests)

set_target_properties(satox-blockchain-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../;$ORIGIN/../"
    BUILD_RPATH "$ORIGIN/../../;$ORIGIN/../"
)
