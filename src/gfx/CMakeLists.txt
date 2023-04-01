add_library(gfx STATIC src/camera.cpp)

find_package(glm CONFIG REQUIRED)
target_link_libraries(gfx PUBLIC glm::glm)

target_include_directories(gfx PUBLIC include ${GLEW_INCLUDE_DIRS})