# JavaScript Bindings CMakeLists.txt

if(BUILD_NODEJS_BINDINGS)
    message(STATUS "Configuring JavaScript bindings...")
    
    # Check for Node.js and npm
    find_program(NODE_EXECUTABLE node)
    find_program(NPM_EXECUTABLE npm)
    
    if(NODE_EXECUTABLE AND NPM_EXECUTABLE)
        message(STATUS "Node.js found: ${NODE_EXECUTABLE}")
        message(STATUS "npm found: ${NPM_EXECUTABLE}")
        
        # Install dependencies
        add_custom_target(javascript-install-deps
            COMMAND ${NPM_EXECUTABLE} install
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Installing JavaScript dependencies"
        )
        
        # Build JavaScript
        add_custom_target(javascript-build
            COMMAND ${NPM_EXECUTABLE} run build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS javascript-install-deps
            COMMENT "Building JavaScript bindings"
        )
        
        # Run tests
        add_custom_target(javascript-test
            COMMAND ${NPM_EXECUTABLE} test
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS javascript-build
            COMMENT "Running JavaScript tests"
        )
        
        message(STATUS "JavaScript bindings configured successfully")
    else()
        message(WARNING "Node.js or npm not found - JavaScript bindings will be skipped")
    endif()
else()
    message(STATUS "JavaScript bindings disabled")
endif() 