if(BUILD_TESTS)
add_executable(satox-wallet-tests
    wallet_manager_test.cpp
)

target_link_libraries(satox-wallet-tests
    PRIVATE
    satox-wallet
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(satox-wallet-tests)

set_target_properties(satox-wallet-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
endif()
