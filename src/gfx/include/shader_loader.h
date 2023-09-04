/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_SHADER_LOADER_H
#define SALMIAC_SHADER_LOADER_H

#include "shader_program.h"

#include <string>

namespace sal {

class Shader_loader {
public:
    static Shader_program from_sources(std::string const& vert_source,
                                       std::string const& frag_source,
                                       std::initializer_list<std::string> const attrib_list,
                                       std::vector<std::string> const& uniform_list) noexcept;

private:
    static void compile_shader(Shader_program& shader,
                               std::string const& file,
                               GLuint& shader_id) noexcept;

    static void add_attrib(Shader_program& shader, std::string const& attrib_name) noexcept;

    static void link_shaders(Shader_program& shader) noexcept;

    static void check_error(bool const check_program_error, Shader_program& shader) noexcept;
};

} // namespace sal

#endif //SALMIAC_SHADER_LOADER_H
