cmake_minimum_required(VERSION 3.14)
project(satox-ipfs VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add library target
add_library(satox-ipfs
    src/ipfs_manager.cpp
)

# Set include directories
target_include_directories(satox-ipfs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-ipfs
    PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        fmt::fmt
)

# Set compile definitions
target_compile_definitions(satox-ipfs
    PRIVATE
        SATOX_IPFS_EXPORTS
    PUBLIC
        SATOX_IPFS_IMPORTS
)

# Add PIC flags for shared library compatibility
target_compile_options(satox-ipfs PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC>
)

# Install rules
include(GNUInstallDirs)
install(TARGETS satox-ipfs
    EXPORT satox-ipfs-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT satox-ipfs-targets
    FILE satox-ipfs-targets.cmake
    NAMESPACE satox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-ipfs
)

# Create and install config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-ipfs-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-ipfs-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-ipfs-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-ipfs
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-ipfs-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-ipfs-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-ipfs
)

# Enable testing
enable_testing()
add_subdirectory(tests) 