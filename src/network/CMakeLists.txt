cmake_minimum_required(VERSION 3.14)
project(satox-network VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(CURL REQUIRED)
# find_package(GTest REQUIRED)  # Using FetchContent instead
find_package(Boost REQUIRED COMPONENTS system)

# Add source files
set(SOURCES
    src/network_manager.cpp
    src/letsencrypt_manager.cpp
)

# Add header files
set(HEADERS
    include/satox/network/network_manager.hpp
    include/satox/network/letsencrypt_manager.hpp
)

# Create library
add_library(satox-network SHARED ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(satox-network
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-network
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
        pthread
    INTERFACE
        spdlog::spdlog
        fmt::fmt
)

# Enable testing
enable_testing()

# Add test executable
add_executable(satox-network-tests
    tests/network_manager_test.cpp
)

# Link test executable with libraries
target_link_libraries(satox-network-tests
    PRIVATE
        spdlog::spdlog
        fmt::fmt
        satox-network
        gtest
        gtest_main
)

# Add tests to CTest
add_test(NAME satox-network-tests COMMAND satox-network-tests)

# Install rules
install(TARGETS satox-network
    EXPORT satox-network-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT satox-network-targets
    FILE satox-network-targets.cmake
    NAMESPACE satox::
    DESTINATION lib/cmake/satox-network
)

# Export targets for use in other CMake projects
export(EXPORT satox-network-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/satox-network-targets.cmake
    NAMESPACE satox::
) 