# Enable testing
enable_testing()

add_executable(satox-wallet-tests
    wallet_manager_test.cpp
)

target_include_directories(satox-wallet-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

target_link_libraries(satox-wallet-tests
    PRIVATE
    satox-wallet
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
)

# Add test to CTest
add_test(NAME satox-wallet-tests COMMAND satox-wallet-tests)

set_target_properties(satox-wallet-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
