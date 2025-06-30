cmake_minimum_required(VERSION 3.10)
project(satox_benchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SatoxSDK REQUIRED)
find_package(benchmark REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Add the benchmark executable
add_executable(sdk_benchmarks sdk_benchmarks.cpp)

# Link against required libraries
target_link_libraries(sdk_benchmarks
    PRIVATE
    SatoxSDK::Core
    SatoxSDK::Blockchain
    SatoxSDK::Asset
    SatoxSDK::NFT
    SatoxSDK::IPFS
    SatoxSDK::Security
    benchmark::benchmark
    benchmark::benchmark_main
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# Set include directories
target_include_directories(sdk_benchmarks
    PRIVATE
    ${SATOX_SDK_INCLUDE_DIRS}
    ${benchmark_INCLUDE_DIRS}
)

# Set compile definitions
target_compile_definitions(sdk_benchmarks
    PRIVATE
    SATOX_MAINNET
)

# Set benchmark options
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_INSTALL OFF) 