# Enable testing features in the core library
set(SATOX_CORE_TESTING ON)

# Define the test executable
add_executable(satox-core-tests
    satox_manager_test.cpp
    core_manager_test.cpp
    wallet_manager_test.cpp
    ipfs_manager_test.cpp
    network_manager_test.cpp
    asset_manager_test.cpp
    blockchain_manager_test.cpp
    security_manager_test.cpp
)

target_include_directories(satox-core-tests PRIVATE /usr/local/include)

# Define TESTING macro for test-only features and for linked libraries
target_compile_definitions(satox-core-tests PUBLIC TESTING SATOX_CORE_TESTING)

# Link the test executable against its dependencies using FetchContent GoogleTest
target_link_libraries(satox-core-tests
    PRIVATE
        satox-core
        gtest
        gtest_main
        Threads::Threads
        gflags
)

# Ensure the test executable can access test-only methods
# target_compile_definitions(satox-core-tests PRIVATE 
#     TESTING
#     SATOX_CORE_TESTING
# )

# Discover and add tests to CTest
include(GoogleTest)

set_target_properties(satox-core-tests PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../;$ORIGIN/../../"
    BUILD_RPATH "$ORIGIN;$ORIGIN/../;$ORIGIN/../../"
)
