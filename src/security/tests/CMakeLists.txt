if(BUILD_TESTS)
add_executable(security_tests
    security_manager_test.cpp
)

target_link_libraries(security_tests
    PRIVATE
    satox-security
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
    fmt::fmt
)

include(GoogleTest)
gtest_discover_tests(security_tests)

set_target_properties(security_tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../;$ORIGIN/../"
    BUILD_RPATH "$ORIGIN/../../;$ORIGIN/../"
)
endif()
