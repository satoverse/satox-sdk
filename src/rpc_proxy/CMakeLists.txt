# MIT License
# Copyright (c) 2025 Satoxcoin Core Developer

cmake_minimum_required(VERSION 3.14)
project(satox-rpc-proxy)

add_library(satox-rpc-proxy STATIC
    rpc_proxy_manager.cpp
    utils.cpp
)

# Include directories
target_include_directories(satox-rpc-proxy PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/satox/rpc_proxy>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/satox/rpc_proxy>
)

# Dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries(satox-rpc-proxy
    PUBLIC
        spdlog::spdlog
        fmt::fmt
        nlohmann_json::nlohmann_json
)

# Add PIC flags for shared library compatibility
target_compile_options(satox-rpc-proxy PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC>
)

# Install
install(TARGETS satox-rpc-proxy DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/satox/rpc_proxy DESTINATION include/satox) 