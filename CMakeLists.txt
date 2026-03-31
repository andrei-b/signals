cmake_minimum_required(VERSION 3.22)
project(untitled)

set(CMAKE_CXX_STANDARD 23)

add_executable(untitled main.cpp
        Connection.cpp
        Connection.h
        Signal.cpp
        Signal.h)

# --- Google Test ---
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(signal_tests
        Signal_test.cpp
        Connection.cpp
        Connection.h
        Signal.h)

target_link_libraries(signal_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(signal_tests)

