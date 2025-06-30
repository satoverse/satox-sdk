cmake_minimum_required(VERSION 3.10)
project(satox-quantum VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)

# Find optional packages
find_package(sodium QUIET)
find_package(liboqs QUIET)

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
)

# Add optional include directories
if(SODIUM_FOUND)
    include_directories(${SODIUM_INCLUDE_DIR})
    message(STATUS "Sodium library found - quantum-resistant features enabled")
else()
    message(WARNING "Sodium library not found - quantum-resistant features will be limited")
endif()

if(LIBOQS_FOUND)
    include_directories(${LIBOQS_INCLUDE_DIR})
    message(STATUS "liboqs library found - post-quantum cryptography enabled")
else()
    message(WARNING "liboqs library not found - post-quantum cryptography will be limited")
endif()

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create library
add_library(satox-quantum STATIC ${SOURCES})

# Link libraries
target_link_libraries(satox-quantum
    ${OPENSSL_LIBRARIES}
    ${GTEST_LIBRARIES}
)

# Add optional libraries
if(SODIUM_FOUND)
    target_link_libraries(satox-quantum ${SODIUM_LIBRARIES})
endif()

if(LIBOQS_FOUND)
    target_link_libraries(satox-quantum ${LIBOQS_LIBRARIES})
endif()

# Add tests
set(SODIUM_FOUND ${SODIUM_FOUND} PARENT_SCOPE)
enable_testing()
add_subdirectory(tests) 