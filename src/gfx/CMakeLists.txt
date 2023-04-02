add_library(gfx STATIC
        src/shader_loader.cpp
        src/camera.cpp
        )

find_package(glm CONFIG REQUIRED)
target_link_libraries(gfx PRIVATE glfw GLEW::GLEW util)

target_include_directories(gfx PUBLIC include)