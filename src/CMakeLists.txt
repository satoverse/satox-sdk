find_package(nlohmann_json REQUIRED)
# Satox Core SDK - Source Components

# This file includes all the core C++ components of the Satox SDK.
# It has been autonomously repaired to remove incorrect references to the Game SDK.

add_subdirectory(core)
add_subdirectory(database)
add_subdirectory(blockchain)
add_subdirectory(wallet)
add_subdirectory(ipfs)
add_subdirectory(asset)
add_subdirectory(network)
add_subdirectory(security)
add_subdirectory(nft)
add_subdirectory(rpc_proxy)

# Satox C API shared library for Python and other language bindings
add_library(satox-sdk-c SHARED satox_sdk_c.cpp)

# Main SDK executable
add_executable(satox-sdk main.cpp)

# Set include directories for the C API
set_target_properties(satox-sdk-c PROPERTIES OUTPUT_NAME "satox-sdk-c")
target_include_directories(satox-sdk-c
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/satox-quantum/include
        ${CMAKE_SOURCE_DIR}/satox-transactions/include
        ${CMAKE_SOURCE_DIR}/satox-assets/include
        ${CMAKE_SOURCE_DIR}/satox-api/include
)

# Set include directories for the main executable
target_include_directories(satox-sdk
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/satox-quantum/include
        ${CMAKE_SOURCE_DIR}/satox-transactions/include
        ${CMAKE_SOURCE_DIR}/satox-assets/include
        ${CMAKE_SOURCE_DIR}/satox-api/include
)

# Link all core Satox SDK libraries
# (adjust as needed if some are static only)
target_link_libraries(satox-sdk-c
    PRIVATE
        satox-core
        satox-blockchain
        satox-database
        satox-asset
        satox-security
        satox-nft
        satox-ipfs
        satox-network
        satox-wallet
        satox-quantum
        satox-transactions
        satox-assets
        satox-api
        spdlog::spdlog
        fmt::fmt
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
        pthread
        dl
)

# Link the main executable with all SDK libraries
target_link_libraries(satox-sdk
    PRIVATE
        satox-core
        satox-blockchain
        satox-database
        satox-asset
        satox-security
        satox-nft
        satox-ipfs
        satox-network
        satox-wallet
        satox-quantum
        satox-transactions
        satox-assets
        satox-api
        spdlog::spdlog
        fmt::fmt
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
        pthread
        dl
)

# Install the C API shared library
install(TARGETS satox-sdk-c
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the main executable
install(TARGETS satox-sdk
    RUNTIME DESTINATION bin
)
