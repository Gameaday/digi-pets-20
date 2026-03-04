cmake_minimum_required(VERSION 3.20)

# Find cpp-httplib
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

# Client library
add_library(digipets_client STATIC
    src/digipets_client.cpp
)

target_include_directories(digipets_client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(digipets_client
    PUBLIC
        digipets_common
        httplib::httplib
)

target_compile_features(digipets_client PUBLIC cxx_std_20)

# Example CLI client
add_executable(digipets_cli
    src/cli_main.cpp
)

target_link_libraries(digipets_cli
    PRIVATE
        digipets_client
)

target_compile_features(digipets_cli PRIVATE cxx_std_20)
