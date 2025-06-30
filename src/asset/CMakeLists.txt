cmake_minimum_required(VERSION 3.16)
project(satox-asset VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

# Use bundled GoogleTest instead of system version
# find_package(GTest REQUIRED)  # Commented out to use bundled version

# Source files
set(SOURCES
    src/asset_manager.cpp
)

# Header files
set(HEADERS
    include/satox/asset/asset_manager.hpp
    include/satox/asset/types.hpp
    include/satox/asset/error.hpp
)

# Create library
add_library(satox-asset STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(satox-asset
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set library properties
set_target_properties(satox-asset PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADERS}"
)

# Link libraries
target_link_libraries(satox-asset
    PUBLIC
        spdlog::spdlog
        fmt::fmt
    PRIVATE
        nlohmann_json::nlohmann_json
)

# Compiler flags
target_compile_options(satox-asset PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -fPIC>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -fPIC>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Test files
set(TEST_SOURCES
    tests/asset_manager_test.cpp
    tests/asset_manager_comprehensive_test.cpp
)

# Create test executable
add_executable(satox-asset-tests ${TEST_SOURCES})

# Set include directories for tests - use bundled GoogleTest/GoogleMock from build/_deps
target_include_directories(satox-asset-tests BEFORE PRIVATE
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Remove previous -isystem flags to avoid confusion
# (leave only the correct include directories above)
target_compile_options(satox-asset-tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link test executable - use bundled GoogleTest
target_link_libraries(satox-asset-tests
    satox-asset
    gtest
    gtest_main
)

# Enable testing
enable_testing()
add_test(NAME AssetTests COMMAND satox-asset-tests)

# Example files
set(EXAMPLE_SOURCES
    examples/basic_usage.cpp
)

# Create example executable
add_executable(asset-example ${EXAMPLE_SOURCES})

# Set include directories for example
target_include_directories(asset-example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link example executable
target_link_libraries(asset-example
    satox-asset
)

# Installation
install(TARGETS satox-asset
    EXPORT satox-asset-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Export targets
install(EXPORT satox-asset-targets
    FILE satox-asset-targets.cmake
    NAMESPACE satox::
    DESTINATION lib/cmake/satox-asset
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    satox-asset-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-asset-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/satox-asset-config.cmake
    INSTALL_DESTINATION lib/cmake/satox-asset
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/satox-asset-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/satox-asset-config-version.cmake
    DESTINATION lib/cmake/satox-asset
) 