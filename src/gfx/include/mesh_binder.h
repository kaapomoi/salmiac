/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_MESH_BINDER_H
#define SALMIAC_MESH_BINDER_H

#include "mesh.h"

namespace sal {

class Mesh_binder {
public:
    static void setup(Mesh& mesh) noexcept;

    static void set_buffer_data(Mesh& mesh) noexcept;

    static void clear_buffer_data(Mesh& mesh) noexcept;
};

} // namespace sal

#endif //SALMIAC_MESH_BINDER_H
