/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_MODEL_H
#define SALMIAC_MODEL_H

#include "mesh.h"

namespace sal {

struct Model {
    std::vector<Mesh> meshes;
    glm::mat4 model_matrix{1.0f};
};

} // namespace sal

#endif //SALMIAC_MODEL_H
