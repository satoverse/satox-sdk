cmake_minimum_required(VERSION 3.14)

project(satox-transactions VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Create library target
add_library(satox-transactions
    src/transaction_manager.cpp
    src/transaction_validator.cpp
    src/transaction_signer.cpp
    src/transaction_broadcaster.cpp
    src/transaction_fee_calculator.cpp
)

# Set include directories
target_include_directories(satox-transactions
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-transactions
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

# Set compile definitions
target_compile_definitions(satox-transactions
    PUBLIC
        SATOX_TRANSACTIONS_EXPORT
    PRIVATE
        SATOX_TRANSACTIONS_IMPORT
)

# Installation rules
install(TARGETS satox-transactions
    EXPORT satox-transactions-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

# Export targets
install(EXPORT satox-transactions-targets
    FILE satox-transactions-targets.cmake
    NAMESPACE satox::
    DESTINATION lib/cmake/satox-transactions
)

# Create and install configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-transactions-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-transactions-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-transactions-config.cmake"
    INSTALL_DESTINATION lib/cmake/satox-transactions
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-transactions-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-transactions-config-version.cmake"
    DESTINATION lib/cmake/satox-transactions
)

# Enable testing
enable_testing()
add_subdirectory(tests) 