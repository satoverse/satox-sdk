# Go Bindings CMakeLists.txt

if(BUILD_GO_BINDINGS)
    message(STATUS "Configuring Go bindings...")
    
    # Check for Go
    find_program(GO_EXECUTABLE go)
    
    if(GO_EXECUTABLE)
        message(STATUS "Go found: ${GO_EXECUTABLE}")
        
        # Install dependencies
        add_custom_target(go-install-deps
            COMMAND ${GO_EXECUTABLE} mod download
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Installing Go dependencies"
        )
        
        # Build Go
        add_custom_target(go-build
            COMMAND ${GO_EXECUTABLE} build -o satox_sdk.so -buildmode=c-shared .
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS go-install-deps
            COMMENT "Building Go bindings"
        )
        
        # Run tests
        add_custom_target(go-test
            COMMAND ${GO_EXECUTABLE} test -v ./...
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS go-build
            COMMENT "Running Go tests"
        )
        
        message(STATUS "Go bindings configured successfully")
    else()
        message(WARNING "Go not found - Go bindings will be skipped")
    endif()
else()
    message(STATUS "Go bindings disabled")
endif() 