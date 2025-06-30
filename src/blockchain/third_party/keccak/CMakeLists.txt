cmake_minimum_required(VERSION 3.15)
project(keccak)

# Source files
set(KECCAK_SOURCES
    src/KeccakF-1600-reference.c
    src/KeccakSponge.c
)

set(KECCAK_HEADERS
    include/KeccakF-1600-reference.h
    include/KeccakSponge.h
)

# Create library
add_library(keccak STATIC ${KECCAK_SOURCES})

# Add PIC flags for shared library compatibility
target_compile_options(keccak PRIVATE
    $<$<C_COMPILER_ID:GNU>:-fPIC>
    $<$<C_COMPILER_ID:Clang>:-fPIC>
)

target_include_directories(keccak
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Install rules
install(TARGETS keccak
    EXPORT keccak-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT keccak-targets
    FILE keccak-targets.cmake
    NAMESPACE keccak::
    DESTINATION lib/cmake/keccak
) 