cmake_minimum_required(VERSION 3.15)
project(satox-blockchain VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set RPATH settings
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
# find_package(GTest REQUIRED)  # Using FetchContent instead
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)

# Try to find fmt using different methods
find_package(fmt REQUIRED)
if(NOT fmt_FOUND)
    # Try alternative method
    find_package(fmt CONFIG REQUIRED)
endif()
if(NOT fmt_FOUND)
    # Try pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FMT REQUIRED fmt)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CURL_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# Add Keccak subdirectory
add_subdirectory(third_party/keccak)

# Source files
set(BLOCKCHAIN_SOURCES
    src/blockchain_manager.cpp
    src/block.cpp
    src/transaction.cpp
    src/kawpow.cpp
    src/utils.cpp
)

set(BLOCKCHAIN_HEADERS
    include/satox/blockchain/blockchain_manager.hpp
    include/satox/blockchain/block.hpp
    include/satox/blockchain/transaction.hpp
    include/satox/blockchain/kawpow.hpp
    include/satox/blockchain/types.hpp
    include/satox/blockchain/error.hpp
    include/satox/blockchain/exceptions.hpp
)

# Add library target
add_library(satox-blockchain SHARED
    src/blockchain_manager.cpp
    src/block.cpp
    src/kawpow.cpp
    src/transaction.cpp
)

# Set include directories
target_include_directories(satox-blockchain
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-blockchain
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
    INTERFACE
        spdlog::spdlog
        fmt::fmt
)

# Link fmt based on what was found
if(fmt_FOUND)
    target_link_libraries(satox-blockchain PUBLIC fmt::fmt)
elseif(FMT_FOUND)
    target_link_libraries(satox-blockchain PUBLIC ${FMT_LIBRARIES})
    target_include_directories(satox-blockchain PRIVATE ${FMT_INCLUDE_DIRS})
    target_compile_options(satox-blockchain PRIVATE ${FMT_CFLAGS_OTHER})
endif()

# Add keccak library
target_link_libraries(satox-blockchain PUBLIC keccak)

# Set compile definitions
target_compile_definitions(satox-blockchain
    PUBLIC
        SATOX_BLOCKCHAIN_EXPORT
    PRIVATE
        SATOX_BLOCKCHAIN_IMPORT
)

# Enable testing
enable_testing()

# Add test executable
add_executable(blockchain_manager_test
    tests/blockchain_manager_test.cpp
)

# Add TESTING definition for test executable
target_compile_definitions(blockchain_manager_test
    PRIVATE
        TESTING
)

target_link_libraries(blockchain_manager_test
    PRIVATE
        spdlog::spdlog
        fmt::fmt
        satox-blockchain
        gtest
        gtest_main
)

# Link fmt to test executable
if(fmt_FOUND)
    target_link_libraries(blockchain_manager_test PRIVATE fmt::fmt)
elseif(FMT_FOUND)
    target_link_libraries(blockchain_manager_test PRIVATE ${FMT_LIBRARIES})
endif()

# Set RPATH for test executable
set_target_properties(blockchain_manager_test PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE
)

# Add test to CTest
add_test(NAME blockchain_manager_test COMMAND blockchain_manager_test)

# Set test properties
set_tests_properties(blockchain_manager_test PROPERTIES
    ENVIRONMENT "GTEST_COLOR=1"
    TIMEOUT 30
)

# Installation rules
install(TARGETS satox-blockchain
    EXPORT satox-blockchain-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Export targets
install(EXPORT satox-blockchain-targets
    FILE satox-blockchain-targets.cmake
    NAMESPACE satox::
    DESTINATION lib/cmake/satox-blockchain
)

# Create and install configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-blockchain-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-blockchain-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-blockchain-config.cmake"
    INSTALL_DESTINATION lib/cmake/satox-blockchain
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-blockchain-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-blockchain-config-version.cmake"
    DESTINATION lib/cmake/satox-blockchain
)

# Tests
add_subdirectory(tests)

# Add example executable
add_executable(blockchain_example
    examples/basic_usage.cpp
)

target_link_libraries(blockchain_example
    PRIVATE
        spdlog::spdlog
        fmt::fmt
        satox-blockchain
) 