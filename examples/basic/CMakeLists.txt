# Basic Examples CMakeLists.txt

# Basic game example
add_executable(basic_game_example
    basic_game_example.cpp
)

target_link_libraries(basic_game_example
    satox-sdk::satox-sdk
)

# Install example
install(TARGETS basic_game_example
    RUNTIME DESTINATION bin
)

