cmake_minimum_required(VERSION 3.14)
project(satox-nft VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

# Add library target
add_library(satox-nft
    src/nft_manager.cpp
)

# Set include directories
target_include_directories(satox-nft
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-nft
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        spdlog::spdlog
        fmt::fmt
)

# Set compile definitions
target_compile_definitions(satox-nft
    PRIVATE
        SATOX_NFT_EXPORTS
    PUBLIC
        SATOX_NFT_IMPORTS
)

# Add PIC flags for shared library compatibility
target_compile_options(satox-nft PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC>
)

# Install rules
include(GNUInstallDirs)
install(TARGETS satox-nft
    EXPORT satox-nft-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT satox-nft-targets
    FILE satox-nft-targets.cmake
    NAMESPACE satox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-nft
)

# Create and install config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-nft-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-nft-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-nft-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-nft
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-nft-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-nft-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-nft
)

# Enable testing
enable_testing()
add_subdirectory(tests) 