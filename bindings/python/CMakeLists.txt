cmake_minimum_required(VERSION 3.16)
project(satox_sdk_python)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python and pybind11
find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

# Find required libraries
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(CURL REQUIRED)
find_package(Eigen3 QUIET)

# Include directories - fixed to properly resolve headers
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core/include
    ${CMAKE_SOURCE_DIR}/src/network/include
    ${CMAKE_SOURCE_DIR}/src/database/include
    ${CMAKE_SOURCE_DIR}/src/security/include
    ${CMAKE_SOURCE_DIR}/src/wallet/include
    ${CMAKE_SOURCE_DIR}/src/asset/include
    ${CMAKE_SOURCE_DIR}/src/nft/include
    ${CMAKE_SOURCE_DIR}/src/blockchain/include
    ${CMAKE_SOURCE_DIR}/src/ipfs/include
    ${CMAKE_SOURCE_DIR}/satox-quantum/include
    ${CMAKE_SOURCE_DIR}/satox-transactions/include
    ${CMAKE_SOURCE_DIR}/satox-assets/include
    ${CMAKE_SOURCE_DIR}/satox-api/include
    /usr/include/eigen3
)

# Link directories
link_directories(
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/core
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/database
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/security
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/wallet
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/asset
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/nft
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/blockchain
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/ipfs
    ${CMAKE_SOURCE_DIR}/../../build_debug/src/network
)

# Python binding source files
set(PYTHON_BINDING_SOURCES
    src/satox_sdk_python.cpp
)

# Create Python module
pybind11_add_module(satox_sdk_python ${PYTHON_BINDING_SOURCES})

# Set target include directories properly
target_include_directories(satox_sdk_python
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core/include
        ${CMAKE_SOURCE_DIR}/src/network/include
        ${CMAKE_SOURCE_DIR}/src/database/include
        ${CMAKE_SOURCE_DIR}/src/security/include
        ${CMAKE_SOURCE_DIR}/src/wallet/include
        ${CMAKE_SOURCE_DIR}/src/asset/include
        ${CMAKE_SOURCE_DIR}/src/nft/include
        ${CMAKE_SOURCE_DIR}/src/blockchain/include
        ${CMAKE_SOURCE_DIR}/src/ipfs/include
        ${CMAKE_SOURCE_DIR}/satox-quantum/include
        ${CMAKE_SOURCE_DIR}/satox-transactions/include
        ${CMAKE_SOURCE_DIR}/satox-assets/include
        ${CMAKE_SOURCE_DIR}/satox-api/include
)

# Link libraries
target_link_libraries(satox_sdk_python
    PRIVATE
        satox-core
        satox-database
        satox-security
        satox-wallet
        satox-asset
        satox-nft
        satox-blockchain
        satox-ipfs
        satox-network
        satox-transactions
        satox-quantum
        satox-assets
        satox-api
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        fmt::fmt
        CURL::libcurl
        pthread
        dl
)

# Set output directory
set_target_properties(satox_sdk_python PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/satox_sdk
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/satox_sdk
)

# Copy the compiled module to the package directory
add_custom_command(TARGET satox_sdk_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:satox_sdk_python>
        ${CMAKE_SOURCE_DIR}/satox_sdk/_satox_sdk_python${PYTHON_MODULE_EXTENSION}
)

# Install target
install(TARGETS satox_sdk_python
    LIBRARY DESTINATION ${Python_SITEARCH}/satox_sdk
    RUNTIME DESTINATION ${Python_SITEARCH}/satox_sdk
)

# Install Python package
install(DIRECTORY satox_sdk/
    DESTINATION ${Python_SITEARCH}/satox_sdk
    FILES_MATCHING PATTERN "*.py"
)

# Install additional files
install(FILES
    README.md
    setup.py
    DESTINATION ${Python_SITEARCH}/satox_sdk
)

# Print configuration info
message(STATUS "Python bindings configuration:")
message(STATUS "  Python executable: ${Python_EXECUTABLE}")
message(STATUS "  Python version: ${Python_VERSION}")
message(STATUS "  Python include dirs: ${Python_INCLUDE_DIRS}")
message(STATUS "  Python libraries: ${Python_LIBRARIES}")
message(STATUS "  pybind11 version: ${pybind11_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output directory: ${CMAKE_SOURCE_DIR}/satox_sdk")

if(Eigen3_FOUND)
    message(STATUS "  Eigen3 found: ${Eigen3_VERSION}")
else()
    message(WARNING "Eigen3 not found. Some features may be limited.")
endif()
