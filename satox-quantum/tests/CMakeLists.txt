# Add test executable
if(TARGET satox-quantum AND SODIUM_FOUND)
add_executable(quantum_tests
    quantum_manager_test.cpp
)
target_link_libraries(quantum_tests
    satox-quantum
    GTest::GTest
    GTest::Main
)
add_test(NAME quantum_tests COMMAND quantum_tests) 
else()
    message(WARNING "Skipping quantum_tests: sodium library not found.")
endif() 