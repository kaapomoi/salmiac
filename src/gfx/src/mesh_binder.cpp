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

    /// TODO: Auto-generate attributes based on Vertex Type


    auto const uv_offset = nullptr;
    auto const normal_offset = (void*)(offsetof(Vertex, normal));
    auto const position_offset = (void*)(offsetof(Vertex, position));
    auto const color_offset = (void*)(offsetof(Vertex, color));

    // vertex texture coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), uv_offset);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), normal_offset);

    // vertex positions
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), position_offset);

    // vertex colors
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), color_offset);

    glBindVertexArray(0);
    mesh.initialized = true;
}

void Mesh_binder::set_buffer_data(Mesh& mesh) noexcept
{
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

/// TODO: Fix this. GL_ERR "Buffer object must be bound"
void Mesh_binder::clear_buffer_data(Mesh& mesh) noexcept
{
    /// TODO: only check what is needed
    if (mesh.vao && mesh.vbo && mesh.ebo) {
        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
}

} // namespace sal
