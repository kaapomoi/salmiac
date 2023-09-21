/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TRANSFORM_H
#define SALMIAC_TRANSFORM_H

#include "glm/glm.hpp"

namespace sal {

struct Transform {
    glm::vec3 position{0.f};
    glm::vec3 rotation{0.f};
    glm::vec3 scale{0.f};

    bool dirty{true};
};

} // namespace sal

#endif //SALMIAC_TRANSFORM_H
