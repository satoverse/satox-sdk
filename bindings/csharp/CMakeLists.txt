# C# Bindings CMakeLists.txt

if(BUILD_CSHARP_BINDINGS)
    message(STATUS "Configuring C# bindings...")
    
    # Check for .NET
    find_program(DOTNET_EXECUTABLE dotnet)
    
    if(DOTNET_EXECUTABLE)
        message(STATUS ".NET found: ${DOTNET_EXECUTABLE}")
        
        # Restore dependencies
        add_custom_target(csharp-restore
            COMMAND ${DOTNET_EXECUTABLE} restore
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Restoring C# dependencies"
        )
        
        # Build C#
        add_custom_target(csharp-build
            COMMAND ${DOTNET_EXECUTABLE} build --configuration Release
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS csharp-restore
            COMMENT "Building C# bindings"
        )
        
        # Run tests
        add_custom_target(csharp-test
            COMMAND ${DOTNET_EXECUTABLE} test --configuration Release
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS csharp-build
            COMMENT "Running C# tests"
        )
        
        message(STATUS "C# bindings configured successfully")
    else()
        message(WARNING ".NET not found - C# bindings will be skipped")
    endif()
else()
    message(STATUS "C# bindings disabled")
endif() 