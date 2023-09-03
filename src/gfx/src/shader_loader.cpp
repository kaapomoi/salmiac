/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "shader_loader.h"

#include "log.h"

namespace sal {

Shader_program
Shader_loader::from_sources(std::string const& vert_source,
                            std::string const& frag_source,
                            std::initializer_list<std::string> const attrib_list) noexcept
{
    Shader_program shader;

    shader.program_id = glCreateProgram();

    auto create_and_compile_shader = [&shader](std::int32_t const gl_shader_type, GLuint& shader_id,
                                               std::string const& shader_source) -> void {
        shader_id = glCreateShader(gl_shader_type);
        if (shader_id == 0) {
            Log::warn("{} shader could not be created!", gl_shader_type);
        }
        compile_shader(shader, shader_source, shader_id);
    };

    create_and_compile_shader(GL_VERTEX_SHADER, shader.vertex_shader_id, vert_source);
    create_and_compile_shader(GL_FRAGMENT_SHADER, shader.fragment_shader_id, frag_source);

    for (auto const& attrib : attrib_list) {
        add_attrib(shader, attrib);
    }

    link_shaders(shader);

    return shader;
}

void Shader_loader::compile_shader(Shader_program& shader,
                                   std::string const& shader_source,
                                   GLuint& shader_id) noexcept
{
    char const* const contents_ptr = shader_source.c_str();
    glShaderSource(shader_id, 1, &contents_ptr, nullptr);

    glCompileShader(shader_id);

    GLint compile_success = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

    if (compile_success == GL_FALSE) {
        check_error(false, shader);

        glDeleteShader(shader_id);
    }
}

void Shader_loader::add_attrib(Shader_program& shader, std::string const& attrib_name) noexcept
{
    glBindAttribLocation(shader.program_id, shader.attrib_count, attrib_name.c_str());
    shader.attrib_count++;
}

void Shader_loader::link_shaders(Shader_program& shader) noexcept
{
    glAttachShader(shader.program_id, shader.vertex_shader_id);
    glAttachShader(shader.program_id, shader.fragment_shader_id);

    glLinkProgram(shader.program_id);

    GLint link_success = 0;
    glGetProgramiv(shader.program_id, GL_LINK_STATUS, static_cast<std::int32_t*>(&link_success));
    if (link_success == GL_FALSE) {
        check_error(true, shader);

        glDeleteProgram(shader.program_id);
        glDeleteShader(shader.vertex_shader_id);
        glDeleteShader(shader.fragment_shader_id);
    }

    glDetachShader(shader.program_id, shader.vertex_shader_id);
    glDetachShader(shader.program_id, shader.fragment_shader_id);
    glDeleteShader(shader.vertex_shader_id);
    glDeleteShader(shader.fragment_shader_id);
}

void Shader_loader::check_error(bool const check_program_error, Shader_program& shader) noexcept
{
    GLint max_length{512};
    if (check_program_error) {
        glGetProgramiv(shader.program_id, GL_INFO_LOG_LENGTH, &max_length);
    }
    else {
        glGetShaderiv(shader.program_id, GL_INFO_LOG_LENGTH, &max_length);
    }

    std::vector<char> error_log(max_length);
    if (check_program_error) {
        glGetProgramInfoLog(shader.program_id, max_length, &max_length, error_log.data());
    }
    else {
        glGetShaderInfoLog(shader.program_id, max_length, &max_length, error_log.data());
    }

    std::string error_str(error_log.begin(), error_log.end());
    Log::warn("Shaders failed to link: {}", error_str);
}


} // namespace sal
