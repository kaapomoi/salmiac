add_library(gfx STATIC
        src/shader_loader.cpp
        src/model_loader.cpp
        src/camera.cpp
        src/texture_loader.cpp
        src/mesh_binder.cpp)

find_package(Stb REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

message(ASSIMP ${ASSIMP_LIBRARIES})

target_link_libraries(gfx PRIVATE glfw GLEW::GLEW util assimp::assimp)

target_include_directories(gfx PUBLIC include ${Stb_INCLUDE_DIR})