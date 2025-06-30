cmake_minimum_required(VERSION 3.14)
project(hello-world)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SATOX SDK
find_package(SatoxSDK REQUIRED)

# Create executable
add_executable(hello-world main.cpp)

# Link with SATOX SDK
target_link_libraries(hello-world SatoxSDK::SatoxSDK)

# Set compiler flags
target_compile_options(hello-world PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Include directories
target_include_directories(hello-world PRIVATE
    ${SatoxSDK_INCLUDE_DIRS}
)

# Set output directory
set_target_properties(hello-world PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS hello-world
    RUNTIME DESTINATION bin
)

# Print configuration info
message(STATUS "Building Hello World Example")
message(STATUS "SATOX SDK Version: ${SatoxSDK_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}") 