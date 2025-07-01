# Enable testing
enable_testing()

# Define the test executable
add_executable(database_tests
    database_manager_test.cpp
    database_manager_comprehensive_test.cpp
    database_vulnerability_tests.cpp
)

# Set include directories for tests
target_include_directories(database_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
)

# Link the test executable against its dependencies
target_link_libraries(database_tests
    PRIVATE
        satox-database
        gtest
        gtest_main
        gmock
        gmock_main
        Threads::Threads
)

# Add test to CTest
add_test(NAME database_tests COMMAND database_tests)
