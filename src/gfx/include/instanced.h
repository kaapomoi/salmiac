/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_INSTANCED_H
#define SALMIAC_INSTANCED_H

#include "model.h"

namespace sal {

struct Instanced {
    Model& model;
    glm::vec4 color{1.f};
};

} // namespace sal

#endif //SALMIAC_INSTANCED_H
