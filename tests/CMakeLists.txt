cmake_minimum_required(VERSION 3.20)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Compile server-side sources for tests (without main)
add_library(server_test_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/src/pet_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/src/user_manager.cpp
)

target_include_directories(server_test_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../server/include
)

target_link_libraries(server_test_lib
    PUBLIC
        digipets_common
)

# Test executable
add_executable(digipets_tests
    test_pet.cpp
    test_pet_manager.cpp
    test_user.cpp
)

target_link_libraries(digipets_tests
    PRIVATE
        digipets_common
        digipets_client
        server_test_lib
        GTest::gtest_main
)

target_include_directories(digipets_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../server/include
)

include(GoogleTest)
gtest_discover_tests(digipets_tests)
