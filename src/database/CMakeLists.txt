cmake_minimum_required(VERSION 3.14)
project(satox-database VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable AddressSanitizer for debugging memory issues
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
    message(STATUS "AddressSanitizer enabled")
endif()

# Find dependencies
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    # Try to find nlohmann/json header
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
        PATHS
            /usr/include
            /usr/local/include
            /opt/local/include
            /sw/include
    )
    if(NLOHMANN_JSON_INCLUDE_DIR)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
        message(STATUS "Found nlohmann/json at ${NLOHMANN_JSON_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "nlohmann/json not found. Please install nlohmann-json3-dev")
    endif()
endif()

# Find spdlog
find_package(spdlog 1.8.0 QUIET)
if(NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found. Please install libspdlog-dev")
endif()

# Find fmt (required by spdlog)
find_package(fmt QUIET)
if(NOT fmt_FOUND)
    message(FATAL_ERROR "fmt not found. Please install libfmt-dev")
endif()

# Add library target
add_library(satox-database SHARED
    src/database_manager.cpp
)

# Set include directories
target_include_directories(satox-database
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-database
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
    INTERFACE
        spdlog::spdlog
        fmt::fmt
)

# Set compile definitions
target_compile_definitions(satox-database
    PRIVATE
        SATOX_DATABASE_EXPORTS
    PUBLIC
        SATOX_DATABASE_IMPORTS
)

# Install rules
include(GNUInstallDirs)
install(TARGETS satox-database
    EXPORT satox-database-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT satox-database-targets
    FILE satox-database-targets.cmake
    NAMESPACE satox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-database
)

# Create and install config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-database-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-database-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-database-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-database
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-database-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-database-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-database
)

# Enable testing
enable_testing()
add_subdirectory(tests) 