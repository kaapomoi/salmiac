/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "mesh_binder.h"

#include "GL/glew.h"

namespace sal {

void Mesh_binder::setup(Mesh& mesh) noexcept
{
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);

    auto const uv_offset = nullptr;
    auto const normal_offset = (void*)(offsetof(Vertex, normal));
    auto const position_offset = (void*)(offsetof(Vertex, position));

    // vertex texture coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), uv_offset);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), normal_offset);

    // vertex positions
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), position_offset);

    glBindVertexArray(0);
}

} // namespace sal
