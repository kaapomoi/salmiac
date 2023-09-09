find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)

add_library(gfx STATIC
        src/shader_loader.cpp
        src/model_loader.cpp
        src/camera.cpp
        src/texture_loader.cpp
        src/primitives.cpp
        src/mesh_binder.cpp)

find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)

message(ASSIMP ${ASSIMP_LIBRARIES})

target_link_libraries(gfx PUBLIC glfw GLEW::GLEW util assimp::assimp)

target_include_directories(gfx
        PUBLIC
        include
        PRIVATE
        ${glfw_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${Stb_INCLUDE_DIR})
