cmake_minimum_required(VERSION 3.14)
project(satox-core VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Try to find prometheus-cpp with fallback
find_package(prometheus-cpp QUIET)
if(NOT prometheus-cpp_FOUND)
    message(WARNING "prometheus-cpp not found via find_package, trying alternative methods...")
    
    # Try to find prometheus-cpp headers directly
    find_path(PROMETHEUS_CPP_INCLUDE_DIR
        NAMES prometheus/counter.h
        PATHS
            /usr/include
            /usr/local/include
            /opt/local/include
            ${CMAKE_CURRENT_SOURCE_DIR}/external/prometheus-cpp/core/include
    )
    
    if(PROMETHEUS_CPP_INCLUDE_DIR)
        message(STATUS "Found prometheus-cpp headers at: ${PROMETHEUS_CPP_INCLUDE_DIR}")
        set(prometheus-cpp_FOUND TRUE)
        add_library(prometheus-cpp::core INTERFACE IMPORTED)
        set_target_properties(prometheus-cpp::core PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${PROMETHEUS_CPP_INCLUDE_DIR}"
        )
    else()
        message(WARNING "prometheus-cpp not found. Monitoring features will be disabled.")
        set(prometheus-cpp_FOUND FALSE)
    endif()
endif()

find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED)

# Database configuration options
option(SATOX_ENABLE_ALL_DATABASES "Enable all database backends (requires external installations)" OFF)
option(SATOX_ENABLE_POSTGRESQL "Enable PostgreSQL support" OFF)
option(SATOX_ENABLE_MYSQL "Enable MySQL support" OFF)
option(SATOX_ENABLE_REDIS "Enable Redis support" OFF)
option(SATOX_ENABLE_MONGODB "Enable MongoDB support" OFF)

# Default database packages (embedded, no external installation required)
find_package(SQLite3 REQUIRED)  # Always required - default database

# Optional RocksDB (embedded, high-performance)
# Use custom-built RocksDB from external directory
set(ROCKSDB_ROOT_DIR ${CMAKE_SOURCE_DIR}/external/rocksdb/build)
set(ROCKSDB_SHARED_LIBRARY ${ROCKSDB_ROOT_DIR}/librocksdb.so)
if(EXISTS ${ROCKSDB_SHARED_LIBRARY})
    set(SATOX_ENABLE_ROCKSDB ON)
    set(ROCKSDB_INCLUDE_DIR ${ROCKSDB_ROOT_DIR})
    message(STATUS "Custom RocksDB shared library found at ${ROCKSDB_ROOT_DIR} - high-performance database enabled")
else()
    message(STATUS "Custom RocksDB shared library not found - SQLite will be used for high-performance operations")
    set(SATOX_ENABLE_ROCKSDB OFF)
endif()

# Optional external databases (only if explicitly requested)
if(SATOX_ENABLE_ALL_DATABASES OR SATOX_ENABLE_POSTGRESQL)
    find_package(PostgreSQL QUIET)
    if(PostgreSQL_FOUND)
        set(SATOX_ENABLE_POSTGRESQL ON)
        message(STATUS "PostgreSQL support enabled")
    else()
        message(WARNING "PostgreSQL requested but not found. Install libpq-dev or disable with -DSATOX_ENABLE_POSTGRESQL=OFF")
        set(SATOX_ENABLE_POSTGRESQL OFF)
    endif()
endif()

if(SATOX_ENABLE_ALL_DATABASES OR SATOX_ENABLE_MYSQL)
    find_package(MySQL QUIET)
    if(MySQL_FOUND)
        set(SATOX_ENABLE_MYSQL ON)
        message(STATUS "MySQL support enabled")
    else()
        message(WARNING "MySQL requested but not found. Install libmysqlclient-dev or disable with -DSATOX_ENABLE_MYSQL=OFF")
        set(SATOX_ENABLE_MYSQL OFF)
    endif()
endif()

if(SATOX_ENABLE_ALL_DATABASES OR SATOX_ENABLE_REDIS)
    find_package(hiredis QUIET)
    if(hiredis_FOUND)
        set(SATOX_ENABLE_REDIS ON)
        message(STATUS "Redis support enabled")
    else()
        message(WARNING "Redis requested but not found. Install libhiredis-dev or disable with -DSATOX_ENABLE_REDIS=OFF")
        set(SATOX_ENABLE_REDIS OFF)
    endif()
endif()

if(SATOX_ENABLE_ALL_DATABASES OR SATOX_ENABLE_MONGODB)
    find_package(mongoc-1.0 QUIET)
    if(mongoc-1.0_FOUND)
        set(SATOX_ENABLE_MONGODB ON)
        message(STATUS "MongoDB support enabled")
    else()
        message(WARNING "MongoDB requested but not found. Install libmongoc-dev or disable with -DSATOX_ENABLE_MONGODB=OFF")
        set(SATOX_ENABLE_MONGODB OFF)
    endif()
endif()

# Print database configuration summary
message(STATUS "=== Satox SDK Database Configuration ===")
message(STATUS "SQLite (default):     ENABLED  (embedded)")
if(SATOX_ENABLE_ROCKSDB)
    message(STATUS "RocksDB (performance): ENABLED  (embedded)")
else()
    message(STATUS "RocksDB (performance): DISABLED (not found)")
endif()
if(SATOX_ENABLE_POSTGRESQL)
    message(STATUS "PostgreSQL:           ENABLED")
else()
    message(STATUS "PostgreSQL:           DISABLED")
endif()
if(SATOX_ENABLE_MYSQL)
    message(STATUS "MySQL:                ENABLED")
else()
    message(STATUS "MySQL:                DISABLED")
endif()
if(SATOX_ENABLE_REDIS)
    message(STATUS "Redis:                ENABLED")
else()
    message(STATUS "Redis:                DISABLED")
endif()
if(SATOX_ENABLE_MONGODB)
    message(STATUS "MongoDB:              ENABLED")
else()
    message(STATUS "MongoDB:              DISABLED")
endif()
message(STATUS "==========================================")

# Cloud database configuration
if(CURL_FOUND)
    message(STATUS "Supabase:              ENABLED")
    message(STATUS "Firebase:              ENABLED")
else()
    message(STATUS "Supabase:              DISABLED (CURL not found)")
    message(STATUS "Firebase:              DISABLED (CURL not found)")
endif()

# Add source files
set(SOURCES

    # Cloud database sources
    src/cloud/supabase_manager.cpp
    src/cloud/firebase_manager.cpp
    src/core_manager.cpp
    src/satox_manager.cpp
    src/wallet_manager.cpp
    src/ipfs_manager.cpp
    src/network_manager.cpp
    src/asset_manager.cpp
    src/blockchain_manager.cpp
    src/transaction_manager.cpp
    src/nft_manager.cpp
    src/plugin_manager.cpp
    src/event_manager.cpp
    src/config_manager.cpp
    src/cache_manager.cpp
    src/logging_manager.cpp
    src/security_manager.cpp
    # src/blockchain.cpp
    # src/blockchain_manager.cpp
    # src/transaction.cpp
    # src/network_config.cpp
    # src/network_interface.cpp
    # src/message_handler.cpp
    # src/performance_optimization.cpp
    # src/api_manager.cpp
    # src/blockchain_manager_impl.cpp
    # src/proof_of_work.cpp
    # src/merkle_tree.cpp
    # src/nft.cpp
    # src/asset.cpp
    # src/quantum_manager.cpp
    # src/quantum.cpp
)

# Add a build option for testing features
option(SATOX_CORE_TESTING "Build core library with testing features" ON)

# Enable testing if BUILD_TESTS is enabled
if(BUILD_TESTS)
    set(SATOX_CORE_TESTING ON)
endif()

# Add header files
set(HEADERS
    include/satox/blockchain/blockchain.hpp
)

# Create library
add_library(satox-core SHARED ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(satox-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(satox-core
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
        dl
        CURL::libcurl
    INTERFACE
        spdlog::spdlog
        fmt::fmt
)

# Link optional database dependencies
if(SATOX_ENABLE_ROCKSDB)
    target_link_libraries(satox-core PUBLIC ${ROCKSDB_SHARED_LIBRARY})
    target_include_directories(satox-core PRIVATE ${ROCKSDB_INCLUDE_DIR})
    target_compile_definitions(satox-core PRIVATE SATOX_ENABLE_ROCKSDB)
endif()

if(SATOX_ENABLE_POSTGRESQL)
    target_link_libraries(satox-core PUBLIC PostgreSQL::PostgreSQL)
    target_compile_definitions(satox-core PRIVATE SATOX_ENABLE_POSTGRESQL)
endif()

if(SATOX_ENABLE_MYSQL)
    target_link_libraries(satox-core PUBLIC MySQL::MySQL)
    target_compile_definitions(satox-core PRIVATE SATOX_ENABLE_MYSQL)
endif()

if(SATOX_ENABLE_REDIS)
    target_link_libraries(satox-core PUBLIC hiredis::hiredis)
    target_compile_definitions(satox-core PRIVATE SATOX_ENABLE_REDIS)
endif()

if(SATOX_ENABLE_MONGODB)
    target_link_libraries(satox-core PUBLIC mongo::mongoc_shared)
    target_compile_definitions(satox-core PRIVATE SATOX_ENABLE_MONGODB)
endif()

# Cloud database support
if(CURL_FOUND)
    target_compile_definitions(satox-core PRIVATE SATOX_ENABLE_CLOUD_DATABASES)
    message(STATUS "Cloud databases (Supabase/Firebase): ENABLED")
else()
    message(WARNING "CURL not found - cloud database support disabled")
    message(STATUS "Cloud databases (Supabase/Firebase): DISABLED")
endif()

# Add TESTING macro when building tests
if(SATOX_CORE_TESTING)
    target_compile_definitions(satox-core PUBLIC TESTING)
endif()

# Set compile definitions
target_compile_definitions(satox-core
    PRIVATE
        SATOX_CORE_EXPORTS
    PUBLIC
        SATOX_CORE_IMPORTS
)

# Add PIC flags for shared library compatibility
target_compile_options(satox-core PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC>
)

# Install rules
include(GNUInstallDirs)
install(TARGETS satox-core
    EXPORT satox-core-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT satox-core-targets
    FILE satox-core-targets.cmake
    NAMESPACE satox::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-core
)

# Create and install config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/satox-core-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/satox-core-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-core-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-core
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/satox-core-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/satox-core-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/satox-core
)

# Enable testing
enable_testing()
add_subdirectory(tests) 