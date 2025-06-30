# Rust Bindings CMakeLists.txt

if(BUILD_RUST_BINDINGS)
    message(STATUS "Configuring Rust bindings...")
    
    # Check for Cargo
    find_program(CARGO_EXECUTABLE cargo)
    
    if(CARGO_EXECUTABLE)
        message(STATUS "Cargo found: ${CARGO_EXECUTABLE}")
        
        # Build Rust
        add_custom_target(rust-build
            COMMAND ${CARGO_EXECUTABLE} build --release
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building Rust bindings"
        )
        
        # Run tests
        add_custom_target(rust-test
            COMMAND ${CARGO_EXECUTABLE} test --verbose
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS rust-build
            COMMENT "Running Rust tests"
        )
        
        # Check formatting
        add_custom_target(rust-fmt-check
            COMMAND ${CARGO_EXECUTABLE} fmt -- --check
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking Rust formatting"
        )
        
        # Run clippy
        add_custom_target(rust-clippy
            COMMAND ${CARGO_EXECUTABLE} clippy -- -D warnings
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running Rust clippy"
        )
        
        message(STATUS "Rust bindings configured successfully")
    else()
        message(WARNING "Cargo not found - Rust bindings will be skipped")
    endif()
else()
    message(STATUS "Rust bindings disabled")
endif() 