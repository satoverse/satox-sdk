# Game SDK Bindings CMakeLists.txt

# This directory will contain language bindings for the Satox SDK
# Currently placeholder - will be expanded as bindings are implemented

message(STATUS "Bindings directory - placeholder for future language bindings")

# Python bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/python/CMakeLists.txt")
    add_subdirectory(python)
else()
    message(STATUS "Python binding not implemented, skipping.")
endif()

# TypeScript bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/typescript/CMakeLists.txt")
    add_subdirectory(typescript)
else()
    message(STATUS "TypeScript binding not implemented, skipping.")
endif()

# JavaScript bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/javascript/CMakeLists.txt")
    add_subdirectory(javascript)
else()
    message(STATUS "JavaScript binding not implemented, skipping.")
endif()

# Go bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/go/CMakeLists.txt")
    add_subdirectory(go)
else()
    message(STATUS "Go binding not implemented, skipping.")
endif()

# Rust bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rust/CMakeLists.txt")
    add_subdirectory(rust)
else()
    message(STATUS "Rust binding not implemented, skipping.")
endif()

# C# bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/csharp/CMakeLists.txt")
    add_subdirectory(csharp)
else()
    message(STATUS "C# binding not implemented, skipping.")
endif()

# Java bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/java/CMakeLists.txt")
    add_subdirectory(java)
else()
    message(STATUS "Java binding not implemented, skipping.")
endif()

# Lua bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lua/CMakeLists.txt")
    add_subdirectory(lua)
else()
    message(STATUS "Lua binding not implemented, skipping.")
endif()

# WASM bindings
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wasm/CMakeLists.txt")
    add_subdirectory(wasm)
else()
    message(STATUS "WASM binding not implemented, skipping.")
endif() 