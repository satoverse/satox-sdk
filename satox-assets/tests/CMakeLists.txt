find_package(GTest REQUIRED)

add_executable(satox-assets-tests
    asset_manager_test.cpp
)

target_link_libraries(satox-assets-tests
    PRIVATE
    satox-assets
    GTest::GTest
    GTest::Main
)

add_test(NAME satox-assets-tests COMMAND satox-assets-tests) 