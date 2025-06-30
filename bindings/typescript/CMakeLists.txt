# TypeScript Bindings CMakeLists.txt

if(BUILD_TYPESCRIPT_BINDINGS)
    message(STATUS "Configuring TypeScript bindings...")
    
    # Check for Node.js and npm
    find_program(NODE_EXECUTABLE node)
    find_program(NPM_EXECUTABLE npm)
    
    if(NODE_EXECUTABLE AND NPM_EXECUTABLE)
        message(STATUS "Node.js found: ${NODE_EXECUTABLE}")
        message(STATUS "npm found: ${NPM_EXECUTABLE}")
        
        # Install dependencies
        add_custom_target(typescript-install-deps
            COMMAND ${NPM_EXECUTABLE} install
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Installing TypeScript dependencies"
        )
        
        # Build TypeScript
        add_custom_target(typescript-build
            COMMAND ${NPM_EXECUTABLE} run build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS typescript-install-deps
            COMMENT "Building TypeScript bindings"
        )
        
        # Run tests
        add_custom_target(typescript-test
            COMMAND ${NPM_EXECUTABLE} test
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS typescript-build
            COMMENT "Running TypeScript tests"
        )
        
        message(STATUS "TypeScript bindings configured successfully")
    else()
        message(WARNING "Node.js or npm not found - TypeScript bindings will be skipped")
    endif()
else()
    message(STATUS "TypeScript bindings disabled")
endif() 