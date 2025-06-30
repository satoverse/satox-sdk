if(BUILD_TESTS)
# Define the test executable
add_executable(database_tests
    database_manager_test.cpp
    database_manager_comprehensive_test.cpp
)

# Link the test executable against its dependencies
target_link_libraries(database_tests
    PRIVATE
        satox-database
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
)

# Discover and add tests to CTest
include(GoogleTest)
gtest_discover_tests(database_tests)
endif()
