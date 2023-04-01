/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#ifndef SALMIAC_VERTEX_H
#define SALMIAC_VERTEX_H

#include "glm/glm.hpp"

namespace sal {

struct Vertex {
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 position;
};

} // namespace sal

#endif //SALMIAC_VERTEX_H
