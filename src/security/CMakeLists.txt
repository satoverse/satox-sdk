cmake_minimum_required(VERSION 3.14)
project(satox-security VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Add library target
add_library(satox-security SHARED
    src/security_manager.cpp
    src/input_validator.cpp
    src/rate_limiter.cpp
)

# Set include directories
target_include_directories(satox-security
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-security
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

# Set linker flags to force linking
set_target_properties(satox-security PROPERTIES
    LINK_FLAGS "-Wl,--no-as-needed -lspdlog -lfmt -pthread"
)

# Set compile definitions
target_compile_definitions(satox-security
    PRIVATE
        SATOX_SECURITY_EXPORTS
    PUBLIC
        SATOX_SECURITY_IMPORTS
)

# Install rules
include(GNUInstallDirs)
install(TARGETS satox-security
    EXPORT satox-security-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT satox-security-targets
    FILE satox-security-targets.cmake
    NAMESPACE satox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-security
)

# Enable testing
enable_testing()
add_subdirectory(tests) 