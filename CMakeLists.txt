cmake_minimum_required(VERSION 4.2)
project(untitled)

set(CMAKE_CXX_STANDARD 23)

add_executable(untitled main.cpp
        Connection.cpp
        Connection.h
        Signal.cpp
        Signal.h)
