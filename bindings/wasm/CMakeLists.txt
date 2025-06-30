# WebAssembly Bindings CMakeLists.txt

if(BUILD_WASM_BINDINGS)
    message(STATUS "Configuring WebAssembly bindings...")
    
    # Check for Emscripten
    find_program(EMCC_EXECUTABLE emcc)
    
    if(EMCC_EXECUTABLE)
        message(STATUS "Emscripten found: ${EMCC_EXECUTABLE}")
        
        # Build WASM
        add_custom_target(wasm-build
            COMMAND ${EMCC_EXECUTABLE} src/satox_sdk.cpp -o satox_sdk.js -s WASM=1 -s EXPORTED_FUNCTIONS='["_main"]' -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building WebAssembly bindings"
        )
        
        # Run tests
        add_custom_target(wasm-test
            COMMAND node tests/test_satox_sdk.js
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS wasm-build
            COMMENT "Running WebAssembly tests"
        )
        
        message(STATUS "WebAssembly bindings configured successfully")
    else()
        message(WARNING "Emscripten not found - WebAssembly bindings will be skipped")
    endif()
else()
    message(STATUS "WebAssembly bindings disabled")
endif() 