cmake_minimum_required(VERSION 3.14)
project(satox-assets VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Set GTest paths explicitly
set(GTEST_ROOT /usr)
set(GTEST_INCLUDE_DIR /usr/include)
set(GTEST_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest.a)
set(GTEST_MAIN_LIBRARY /usr/lib/x86_64-linux-gnu/libgtest_main.a)
find_package(GTest REQUIRED)

# Create library target
add_library(satox-assets
    src/asset_manager.cpp
)

# Set include directories
target_include_directories(satox-assets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-assets
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

# Set compile definitions
target_compile_definitions(satox-assets
    PUBLIC
        SATOX_ASSETS_EXPORT
    PRIVATE
        SATOX_ASSETS_IMPORT
)

# Installation rules
install(TARGETS satox-assets
    EXPORT satox-assets-targets
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
install(EXPORT satox-assets-targets
    FILE satox-assets-targets.cmake
    NAMESPACE satox::
    DESTINATION lib/cmake/satox-assets
)

# Create and install configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-assets-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-assets-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-assets-config.cmake"
    INSTALL_DESTINATION lib/cmake/satox-assets
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-assets-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-assets-config-version.cmake"
    DESTINATION lib/cmake/satox-assets
)

# Enable testing
enable_testing()
add_subdirectory(tests) 