if(BUILD_TESTS)
add_executable(ipfs_tests
    ipfs_manager_test.cpp
)

target_link_libraries(ipfs_tests
    PRIVATE
    satox-ipfs
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(ipfs_tests)

set_target_properties(ipfs_tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
endif()
