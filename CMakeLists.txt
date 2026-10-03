cmake_minimum_required(VERSION 3.16)

project(StreamLens
    VERSION 0.1.0
    DESCRIPTION "A lightweight C++ video stream relay and diagnostics toolkit"
    LANGUAGES CXX
)

option(STREAMLENS_BUILD_TESTS "Build StreamLens tests" ON)
option(STREAMLENS_ENABLE_WARNINGS "Enable compiler warnings" ON)

add_executable(streamlens
    src/config.cpp
    src/main.cpp
    src/network/udp_receiver.cpp
    src/network/udp_sender.cpp
)

target_include_directories(streamlens
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(streamlens PRIVATE cxx_std_17)

if(STREAMLENS_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(streamlens PRIVATE /W4 /permissive-)
    else()
        target_compile_options(streamlens PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

include(CTest)

if(STREAMLENS_BUILD_TESTS AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
