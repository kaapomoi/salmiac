/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef NODE_H
#define NODE_H

#include "model.h"

struct Node {
    glm::vec3 position{0.0f};
    glm::vec3 force{0.0f};
    glm::vec3 acceleration{0.0f};
    glm::vec3 velocity{0.0f};
    float mass{1.0f};
};

#endif
