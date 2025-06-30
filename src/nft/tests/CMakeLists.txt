if(BUILD_TESTS)
add_executable(nft_tests
    nft_manager_test.cpp
)

target_link_libraries(nft_tests
    PRIVATE
    satox-nft
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(nft_tests)

set_target_properties(nft_tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
endif()
