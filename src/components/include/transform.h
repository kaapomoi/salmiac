/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TRANSFORM_H
#define SALMIAC_TRANSFORM_H

#include "glm/glm.hpp"

namespace sal {

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

}; // namespace sal


#endif //SALMIAC_TRANSFORM_H
