cmake_minimum_required(VERSION 3.14)
project(satox_sdk_python)

# Find required packages
find_package(pybind11 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../../../include
    ${CMAKE_SOURCE_DIR}/../../../src
    ${pybind11_INCLUDE_DIRS}
)

# Find the Satox SDK library
# find_library(SATOX_SDK_LIB satox-sdk-c PATHS ${CMAKE_SOURCE_DIR}/../../../build/src NO_DEFAULT_PATH)

# Create the Python module
pybind11_add_module(_satox_sdk
    satox_sdk_bindings.cpp
    satox_sdk_python.cpp
    simple_bindings.cpp
    simple_python_bindings.cpp
    bindings.cpp
    satox_python_bindings.cpp
)

target_link_libraries(_satox_sdk PRIVATE satox-sdk-c ${CMAKE_DL_LIBS})
add_dependencies(_satox_sdk satox-sdk-c)

# Set output directory
set_target_properties(_satox_sdk PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../satox_sdk
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../satox_sdk
)

# Install target
install(TARGETS _satox_sdk
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/satox_sdk
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/satox_sdk
) 