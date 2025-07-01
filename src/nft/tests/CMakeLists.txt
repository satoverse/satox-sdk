# Enable testing
enable_testing()

add_executable(nft_tests
    nft_manager_test.cpp
)

target_include_directories(nft_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

target_link_libraries(nft_tests
    PRIVATE
    satox-nft
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)

# Add test to CTest
add_test(NAME nft_tests COMMAND nft_tests)

set_target_properties(nft_tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
