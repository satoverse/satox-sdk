# Java Bindings CMakeLists.txt

if(BUILD_JAVA_BINDINGS)
    message(STATUS "Configuring Java bindings...")
    
    # Check for Java and Maven
    find_program(JAVA_EXECUTABLE java)
    find_program(JAVAC_EXECUTABLE javac)
    find_program(MAVEN_EXECUTABLE mvn)
    
    if(JAVA_EXECUTABLE AND JAVAC_EXECUTABLE)
        message(STATUS "Java found: ${JAVA_EXECUTABLE}")
        message(STATUS "Javac found: ${JAVAC_EXECUTABLE}")
        
        if(MAVEN_EXECUTABLE)
            message(STATUS "Maven found: ${MAVEN_EXECUTABLE}")
            
            # Build Java
            add_custom_target(java-build
                COMMAND ${MAVEN_EXECUTABLE} clean compile
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Building Java bindings"
            )
            
            # Run tests
            add_custom_target(java-test
                COMMAND ${MAVEN_EXECUTABLE} test
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                DEPENDS java-build
                COMMENT "Running Java tests"
            )
            
            message(STATUS "Java bindings configured successfully")
        else()
            message(WARNING "Maven not found - Java bindings will be skipped")
        endif()
    else()
        message(WARNING "Java or Javac not found - Java bindings will be skipped")
    endif()
else()
    message(STATUS "Java bindings disabled")
endif() 