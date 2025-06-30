# Lua Bindings CMakeLists.txt

if(BUILD_LUA_BINDINGS)
    message(STATUS "Configuring Lua bindings...")
    
    # Check for Lua
    find_program(LUA_EXECUTABLE lua)
    find_program(LUAC_EXECUTABLE luac)
    
    if(LUA_EXECUTABLE AND LUAC_EXECUTABLE)
        message(STATUS "Lua found: ${LUA_EXECUTABLE}")
        message(STATUS "Luac found: ${LUAC_EXECUTABLE}")
        
        # Build Lua module
        add_custom_target(lua-build
            COMMAND ${LUAC_EXECUTABLE} -o satox_sdk.luac src/satox_sdk.lua
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building Lua bindings"
        )
        
        # Run tests
        add_custom_target(lua-test
            COMMAND ${LUA_EXECUTABLE} tests/test_satox_sdk.lua
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS lua-build
            COMMENT "Running Lua tests"
        )
        
        message(STATUS "Lua bindings configured successfully")
    else()
        message(WARNING "Lua not found - Lua bindings will be skipped")
    endif()
else()
    message(STATUS "Lua bindings disabled")
endif() 