if(BUILD_TESTS)
add_executable(satox-asset-tests
    asset_manager_test.cpp
    asset_manager_comprehensive_test.cpp
)

target_link_libraries(satox-asset-tests
    PRIVATE
    satox-asset
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(satox-asset-tests)

set_target_properties(satox-asset-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
endif()
