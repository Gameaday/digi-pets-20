cmake_minimum_required(VERSION 3.20)

# Find cpp-httplib (header-only library)
find_package(httplib QUIET)

if(NOT httplib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        cpp-httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.14.3
    )
    FetchContent_MakeAvailable(cpp-httplib)
endif()

# Server executable
add_executable(digipets_server
    src/main.cpp
    src/pet_manager.cpp
    src/user_manager.cpp
)

target_include_directories(digipets_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(digipets_server
    PRIVATE
        digipets_common
        httplib::httplib
)

target_compile_features(digipets_server PRIVATE cxx_std_20)

# Enable threading
find_package(Threads REQUIRED)
target_link_libraries(digipets_server PRIVATE Threads::Threads)
