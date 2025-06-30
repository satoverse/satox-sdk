if(BUILD_TESTS)
add_executable(satox-network-tests
    network_manager_test.cpp
)

target_link_libraries(satox-network-tests
    PRIVATE
    satox-network
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(satox-network-tests)

set_target_properties(satox-network-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
endif()
