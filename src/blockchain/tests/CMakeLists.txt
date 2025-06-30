if(BUILD_TESTS)
add_executable(satox-blockchain-tests
    blockchain_manager_test.cpp
    block_test.cpp
)

target_link_libraries(satox-blockchain-tests
    PRIVATE
    satox-blockchain
    gtest
    gtest_main
    Threads::Threads
)

include(GoogleTest)
# gtest_discover_tests(satox-blockchain-tests)

set_target_properties(satox-blockchain-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN/../../"
)
endif()
