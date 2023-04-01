/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_SHADER_PROGRAM_H
#define SALMIAC_SHADER_PROGRAM_H

#include "GL/glew.h"

#include <cstdint>

namespace sal {

struct Shader_program {
    GLuint program_id{};
    GLuint vertex_shader_id{};
    GLuint fragment_shader_id{};
    std::int32_t attrib_count{};
};

} // namespace sal

#endif //SALMIAC_SHADER_PROGRAM_H
