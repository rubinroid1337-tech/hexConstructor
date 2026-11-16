cmake_minimum_required(VERSION 3.28)
project(LogicSimulator CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER clang++)

find_package(raylib REQUIRED)

add_executable(simulator src/main.cpp)

target_link_libraries(simulator PRIVATE raylib)
