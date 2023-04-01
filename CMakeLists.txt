cmake_minimum_required(VERSION 3.25)
project(salmiac)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)

add_subdirectory(src)

add_executable(salmiac main.cpp)

target_link_libraries(salmiac PRIVATE glfw GLEW::GLEW gfx util)
target_include_directories(salmiac PRIVATE ${glfw_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
