cmake_minimum_required(VERSION 3.16)
project(satox-sdk LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add custom cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Debug: Print module path
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# --- Dependencies ---
# Find required packages for the entire SDK.
# Components will link to these as needed.
find_package(OpenSSL REQUIRED)

# Try to find spdlog with fallback
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(WARNING "spdlog not found via find_package, trying alternative methods...")
    
    # Try to find spdlog headers directly
    find_path(SPDLOG_INCLUDE_DIR
        NAMES spdlog/spdlog.h
        PATHS
            /usr/include
            /usr/local/include
            /opt/local/include
            ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include
    )
    
    if(SPDLOG_INCLUDE_DIR)
        message(STATUS "Found spdlog headers at: ${SPDLOG_INCLUDE_DIR}")
        set(spdlog_FOUND TRUE)
        add_library(spdlog::spdlog INTERFACE IMPORTED)
        set_target_properties(spdlog::spdlog PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}"
        )
    else()
        message(FATAL_ERROR "spdlog not found. Please install libspdlog-dev or build from source.")
    endif()
endif()

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# --- Doxygen Documentation ---
# find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Set the output directory for Doxygen
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/docs/api)

    # Request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Add the target to run Doxygen
    add_custom_target(docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    # Install documentation
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/api/html
            DESTINATION share/doc/satox-sdk
            OPTIONAL)
else()
    message(WARNING "Doxygen not found. API documentation will not be generated.")
endif()

# --- Subdirectories ---
# Add the source directory always
add_subdirectory(src)

# Add additional SDK components
add_subdirectory(satox-quantum)
add_subdirectory(satox-transactions)
add_subdirectory(satox-assets)
add_subdirectory(satox-api)

# Add bindings
add_subdirectory(bindings)

# Only add tests if BUILD_TESTS is ON
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Only add examples if BUILD_EXAMPLES is ON
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Global Project Settings ---
set(PROJECT_VERSION "1.0.0")
set(PROJECT_VERSION_MAJOR 1)

# --- Options ---
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_NODEJS_BINDINGS "Build Node.js bindings" ON)
option(BUILD_TYPESCRIPT_BINDINGS "Build TypeScript bindings" ON)
option(BUILD_GO_BINDINGS "Build Go bindings" ON)
option(BUILD_RUST_BINDINGS "Build Rust bindings" ON)
option(BUILD_JAVA_BINDINGS "Build Java bindings" ON)
option(BUILD_CSHARP_BINDINGS "Build C# bindings" ON)
option(BUILD_LUA_BINDINGS "Build Lua bindings" ON)
option(BUILD_WASM_BINDINGS "Build WebAssembly bindings" ON)
option(BUILD_PACKAGE "Build package" OFF)
option(BUILD_PERFORMANCE_TESTS "Build performance tests" OFF)
option(BUILD_INTEGRATION_TESTS "Build integration tests" OFF)
option(BUILD_DOCUMENTATION "Build documentation" ON)
option(ENABLE_QUANTUM_SECURITY "Enable quantum-resistant security" ON)
option(ENABLE_LOGGING "Enable logging" ON)
option(ENABLE_MONITORING "Enable monitoring" ON)

# --- Fetch GoogleTest ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

# --- Installation ---
# This section defines how the SDK components will be installed.
# The individual components will define what gets installed.
install(
    DIRECTORY include/
    DESTINATION include
)

# Install additional component headers
install(
    DIRECTORY satox-quantum/include/
    DESTINATION include/satox/quantum
    OPTIONAL
)

install(
    DIRECTORY satox-transactions/include/
    DESTINATION include/satox/transactions
    OPTIONAL
)

install(
    DIRECTORY satox-assets/include/
    DESTINATION include/satox/assets
    OPTIONAL
)

install(
    DIRECTORY satox-api/include/
    DESTINATION include/satox/api
    OPTIONAL
)

# --- Final Message ---
message(STATUS "=== Satox SDK Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Dependencies Found:")
message(STATUS "  - OpenSSL: ${OpenSSL_FOUND}")
message(STATUS "  - spdlog: ${spdlog_FOUND}")
message(STATUS "  - fmt: ${fmt_FOUND}")
message(STATUS "  - CURL: ${CURL_FOUND}")
message(STATUS "  - nlohmann_json: ${nlohmann_json_FOUND}")
message(STATUS "  - Doxygen: ${DOXYGEN_FOUND}")
message(STATUS "GoogleTest: ${googletest_FOUND}")
message(STATUS "Build Options:")
message(STATUS "  - BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "  - BUILD_EXAMPLES: ${BUILD_EXAMPLES}")
message(STATUS "  - ENABLE_QUANTUM_SECURITY: ${ENABLE_QUANTUM_SECURITY}")
message(STATUS "  - ENABLE_LOGGING: ${ENABLE_LOGGING}")
message(STATUS "  - ENABLE_MONITORING: ${ENABLE_MONITORING}")
message(STATUS "Bindings:")
message(STATUS "  - Python: ${BUILD_PYTHON_BINDINGS}")
message(STATUS "  - Node.js: ${BUILD_NODEJS_BINDINGS}")
message(STATUS "  - TypeScript: ${BUILD_TYPESCRIPT_BINDINGS}")
message(STATUS "  - Go: ${BUILD_GO_BINDINGS}")
message(STATUS "  - Rust: ${BUILD_RUST_BINDINGS}")
message(STATUS "  - Java: ${BUILD_JAVA_BINDINGS}")
message(STATUS "  - C#: ${BUILD_CSHARP_BINDINGS}")
message(STATUS "  - Lua: ${BUILD_LUA_BINDINGS}")
message(STATUS "  - WebAssembly: ${BUILD_WASM_BINDINGS}")
message(STATUS "  - Package: ${BUILD_PACKAGE}")
message(STATUS "  - Performance Tests: ${BUILD_PERFORMANCE_TESTS}")
message(STATUS "  - Integration Tests: ${BUILD_INTEGRATION_TESTS}")
message(STATUS "  - Documentation: ${BUILD_DOCUMENTATION}")
message(STATUS "===============================")
