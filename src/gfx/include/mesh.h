/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_MESH_H
#define SALMIAC_MESH_H

#include "texture.h"
#include "vertex.h"

#include <vector>

namespace sal {

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Texture> textures;

    std::uint32_t vao{0};
    std::uint32_t vbo{0};
    std::uint32_t ebo{0};
    bool initialized{false};
};

} // namespace sal

#endif //SALMIAC_MESH_H
