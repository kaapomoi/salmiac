/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_SHADER_PROGRAM_H
#define SALMIAC_SHADER_PROGRAM_H

#include "log.h"

#include "GL/glew.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <string>

namespace sal {

struct Shader_program {
    GLuint program_id{0};
    std::int32_t attrib_count{0};

    void use() noexcept
    {
        glUseProgram(program_id);
        for (std::int32_t i{0}; i < attrib_count; i++) {
            glEnableVertexAttribArray(i);
        }
    }

    void un_use() noexcept
    {
        glUseProgram(0);
        for (std::int32_t i{0}; i < attrib_count; i++) {
            glDisableVertexAttribArray(i);
        }
    }

    template<typename T>
    void set_uniform(std::string const& name, T const& value)
    {
        if constexpr (std::is_same_v<T, std::int32_t>) {
            auto res = glGetUniformLocation(program_id, name.c_str());
            glUniform1i(res, value);
        }
        else if constexpr (std::is_same_v<T, float>) {
            glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
        }
        else if constexpr (std::is_same_v<T, glm::vec3>) {
            glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
        }
        else if constexpr (std::is_same_v<T, glm::mat4>) {
            auto res = glGetUniformLocation(program_id, name.c_str());
            glUniformMatrix4fv(res, 1, GL_FALSE, glm::value_ptr(value));
        }
        else {
            Log::error("Invalid type in set_uniform.");
        }
    }

    friend class Shader_loader;

private:
    GLuint vertex_shader_id{0};
    GLuint fragment_shader_id{0};
};

} // namespace sal

#endif //SALMIAC_SHADER_PROGRAM_H
