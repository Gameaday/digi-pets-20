cmake_minimum_required(VERSION 3.20)

# Find nlohmann_json (header-only library)
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# OpenSSL is required for SHA-256 password hashing
find_package(OpenSSL REQUIRED)

# Common library
add_library(digipets_common STATIC
    src/pet.cpp
    src/user.cpp
)

target_include_directories(digipets_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(digipets_common
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::Crypto
)

target_compile_features(digipets_common PUBLIC cxx_std_20)
