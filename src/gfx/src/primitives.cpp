/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "primitives.h"

#include "mesh_binder.h"

namespace sal {

Mesh Primitive_factory::cube(const glm::vec3 dims, std::vector<Texture> const textures) noexcept
{
    Mesh mesh;

    mesh.vertices = {
        Vertex{glm::vec2(0.625, 0.5), glm::vec3(-0, 1, -0), glm::vec3(0.5, 0.5, -0.5)},
        Vertex{glm::vec2(0.875, 0.5), glm::vec3(-0, 1, -0), glm::vec3(-0.5, 0.5, -0.5)},
        Vertex{glm::vec2(0.875, 0.25), glm::vec3(-0, 1, -0), glm::vec3(-0.5, 0.5, 0.5)},
        Vertex{glm::vec2(0.625, 0.25), glm::vec3(-0, 1, -0), glm::vec3(0.5, 0.5, 0.5)},
        Vertex{glm::vec2(0.375, 0.25), glm::vec3(-0, -0, 1), glm::vec3(0.5, -0.5, 0.5)},
        Vertex{glm::vec2(0.625, 0.25), glm::vec3(-0, -0, 1), glm::vec3(0.5, 0.5, 0.5)},
        Vertex{glm::vec2(0.625, 0), glm::vec3(-0, -0, 1), glm::vec3(-0.5, 0.5, 0.5)},
        Vertex{glm::vec2(0.375, 0), glm::vec3(-0, -0, 1), glm::vec3(-0.5, -0.5, 0.5)},
        Vertex{glm::vec2(0.375, 1), glm::vec3(-1, -0, -0), glm::vec3(-0.5, -0.5, 0.5)},
        Vertex{glm::vec2(0.625, 1), glm::vec3(-1, -0, -0), glm::vec3(-0.5, 0.5, 0.5)},
        Vertex{glm::vec2(0.625, 0.75), glm::vec3(-1, -0, -0), glm::vec3(-0.5, 0.5, -0.5)},
        Vertex{glm::vec2(0.375, 0.75), glm::vec3(-1, -0, -0), glm::vec3(-0.5, -0.5, -0.5)},
        Vertex{glm::vec2(0.125, 0.5), glm::vec3(-0, -1, -0), glm::vec3(-0.5, -0.5, -0.5)},
        Vertex{glm::vec2(0.375, 0.5), glm::vec3(-0, -1, -0), glm::vec3(0.5, -0.5, -0.5)},
        Vertex{glm::vec2(0.375, 0.25), glm::vec3(-0, -1, -0), glm::vec3(0.5, -0.5, 0.5)},
        Vertex{glm::vec2(0.125, 0.25), glm::vec3(-0, -1, -0), glm::vec3(-0.5, -0.5, 0.5)},
        Vertex{glm::vec2(0.375, 0.5), glm::vec3(1, -0, -0), glm::vec3(0.5, -0.5, -0.5)},
        Vertex{glm::vec2(0.625, 0.5), glm::vec3(1, -0, -0), glm::vec3(0.5, 0.5, -0.5)},
        Vertex{glm::vec2(0.625, 0.25), glm::vec3(1, -0, -0), glm::vec3(0.5, 0.5, 0.5)},
        Vertex{glm::vec2(0.375, 0.25), glm::vec3(1, -0, -0), glm::vec3(0.5, -0.5, 0.5)},
        Vertex{glm::vec2(0.375, 0.75), glm::vec3(-0, -0, -1), glm::vec3(-0.5, -0.5, -0.5)},
        Vertex{glm::vec2(0.625, 0.75), glm::vec3(-0, -0, -1), glm::vec3(-0.5, 0.5, -0.5)},
        Vertex{glm::vec2(0.625, 0.5), glm::vec3(-0, -0, -1), glm::vec3(0.5, 0.5, -0.5)},
        Vertex{glm::vec2(0.375, 0.5), glm::vec3(-0, -0, -1), glm::vec3(0.5, -0.5, -0.5)}};

    mesh.indices = {0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
                    12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

    for (auto& vert : mesh.vertices) {
        vert.position *= dims;
    }
    mesh.textures = textures;

    Mesh_binder::setup(mesh);

    return mesh;
}

} // namespace sal
