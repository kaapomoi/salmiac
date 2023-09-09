/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef PRIMITIVE_FACTORY_H
#define PRIMITIVE_FACTORY_H

#include "mesh.h"

namespace sal {

class Primitive_factory {
public:
    static Mesh cube(glm::vec3 const dimensions, std::vector<Texture> const textures = {}) noexcept;

private:
};

} // namespace sal

#endif
