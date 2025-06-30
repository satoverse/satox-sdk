cmake_minimum_required(VERSION 3.14)
project(satox-wallet VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(satox-wallet
    src/address_manager.cpp
    src/key_manager.cpp
    src/wallet_manager.cpp
)

target_include_directories(satox-wallet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(satox-wallet
    PUBLIC
        OpenSSL::Crypto
        spdlog::spdlog
        fmt::fmt
)

# Add PIC flags for shared library compatibility
target_compile_options(satox-wallet PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC>
)

install(TARGETS satox-wallet
    EXPORT satox-wallet-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Enable testing
enable_testing()
add_subdirectory(tests) 