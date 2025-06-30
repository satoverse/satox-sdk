find_package(Threads REQUIRED)

add_executable(satox-transactions-tests
    transaction_manager_test.cpp
    transaction_validator_test.cpp
)

target_link_libraries(satox-transactions-tests
    PRIVATE
    satox-transactions
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
)

add_test(NAME satox-transactions-tests COMMAND satox-transactions-tests) 