# MIT License
# Copyright (c) 2025 Satoxcoin Core Developer

cmake_minimum_required(VERSION 3.14)
project(rpc_proxy_tests)

find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(rpc_proxy_manager_test rpc_proxy_manager_test.cpp)
target_include_directories(rpc_proxy_manager_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/satox/rpc_proxy
)
target_link_libraries(rpc_proxy_manager_test
    PRIVATE
        satox-rpc-proxy
        spdlog::spdlog
        fmt::fmt
        nlohmann_json::nlohmann_json
        GTest::gtest_main
)

enable_testing()
add_test(NAME rpc_proxy_manager_test COMMAND rpc_proxy_manager_test) 