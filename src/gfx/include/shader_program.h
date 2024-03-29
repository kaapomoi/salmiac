/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_SHADER_PROGRAM_H
#define SALMIAC_SHADER_PROGRAM_H

#include "log.h"

#include "GL/glew.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <string>

namespace sal {

struct Shader_program {
    struct Attribute {
        std::string name;
        std::size_t size{1};
    };

    GLuint program_id{0};
    std::int32_t attrib_count{0};

    void use() const noexcept
    {
        glUseProgram(program_id);
        for (std::int32_t i{0}; i < attrib_count; i++) {
            glEnableVertexAttribArray(i);
        }
    }

    void un_use() const noexcept
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
        else if constexpr (std::is_same_v<T, glm::vec4>) {
            glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
        }
        else if constexpr (std::is_same_v<T, glm::mat4>) {
            auto res = glGetUniformLocation(program_id, name.c_str());
            glUniformMatrix4fv(res, 1, GL_FALSE, glm::value_ptr(value));
        }
        else {
            Log::error("Invalid type in set_uniform.");
        }
    }

    [[nodiscard]] bool has_uniform(std::string const& uniform) const noexcept
    {
        return std::find(m_uniforms.begin(), m_uniforms.end(), uniform) != m_uniforms.end();
    }

    friend class Shader_loader;

    Shader_program() noexcept = default;
    Shader_program(Shader_program&& other) noexcept = default;
    Shader_program& operator=(Shader_program&& other) noexcept = default;

    Shader_program(Shader_program& other) noexcept = default;
    Shader_program& operator=(Shader_program const& other) noexcept = default;

private:
    std::vector<std::string> m_uniforms;

    GLuint vertex_shader_id{0};
    GLuint fragment_shader_id{0};
};

} // namespace sal

#endif //SALMIAC_SHADER_PROGRAM_H
