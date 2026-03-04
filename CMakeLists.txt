cmake_minimum_required(VERSION 3.20)
project(DigiPets VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SERVER "Build server" ON)
option(BUILD_CLIENT "Build client library" ON)

# Common library (shared between server and client)
add_subdirectory(common)

# Server executable
if(BUILD_SERVER)
    add_subdirectory(server)
endif()

# Client library
if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
