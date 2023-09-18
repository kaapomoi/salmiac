cmake_minimum_required(VERSION 3.25)
project(salmiac)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
add_subdirectory(demo)

add_executable(salmiac main.cpp)

target_link_libraries(salmiac PRIVATE nbody conquest util)
