/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_MODEL_H
#define SALMIAC_MODEL_H

#include "mesh.h"

namespace sal {

struct Model {
    std::vector<Mesh> meshes;
};

} // namespace sal

#endif //SALMIAC_MODEL_H
