add_library(input STATIC
        src/input_manager.cpp
        )

target_link_libraries(input PRIVATE gfx GLEW::GLEW glfw)

target_include_directories(input PUBLIC include)